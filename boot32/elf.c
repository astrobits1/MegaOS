#include <boot32/elf.h>
#include <boot32/vga.h>
#include <boot32/boot.h>

/* Input: Raw 64 bit ELF64 data, address to load to  */
/* Output: struct containing metadata */

/* Loads ELF64 at given virtual address, disregarding the virtual address the ELF wants
 * its segments to be loaded at, but preserving exact offsets and memory layout.
 * 
 * This leaves the ELF in a non executable state, but the virtual memory it is allocated in
 * can be remapped to the virtual memory address it expects to be in and get it going.
 *
 * This is useful particularly because to load the 64 bit kernel, in x86_64 compatibility mode
 * we cannot access more than 32 bit virtual memory, so loading the kernel in the higher
 * half cannot be normally done as mapping that region is impossible in this state.
 *
 * So we just take some physical address range, map it to identity, load the 64 bit ELF there,
 * set up page maps to actually map the higher half region that it expects by reading the metadata
 * and finally long jumping to its entry point, thus entering long mode simultaneously while
 * loading the 64 bit kernel */

struct elf_metadata load_elf64_exec_at(uint8_t* data, uint32_t size, uint8_t* address) {
    struct elf_metadata meta = {0, 0, 0};

    if (size < sizeof(struct elf_ehdr)) {
        vga_print_color("ELF not big enough\n", VGA_COLOR_RED);
        boot_panic();

        return meta;
    }

    struct elf_ehdr* header = (struct elf_ehdr*)data;
    elft_char* ident = (elft_char*)&header->e_ident;


    if ((ident[0] | ident[1]<<8 | ident[2]<<16 | ident[3]<<24) != 0x464C457F) {
        vga_print_color("ELF header checksum failed\n", VGA_COLOR_RED);
        boot_panic();
        return meta;
    }

    /* 64 bit, executable */
    if ((ident[4] | header->e_type << 8) != 0x0202) {
        vga_print_color("ELF must be a 64 bit executable\n", VGA_COLOR_RED);
        boot_panic();
        return meta;
    }

    elft_off pht_offset = header->e_phoff;
    elft_half pht_entry_size = header->e_phentsize;
    elft_half pht_entry_count = header->e_phnum;

    uint64_t virtual_start = UINT64_MAX;
    uint64_t virtual_end = 0;

    /* Find top and bottom level virtual pointer, this is then patched at the start 
     * of the address we are given and offsets are calculated */
    for (elft_half i=0; i<pht_entry_count; i++) {
        struct elf_phdr* entry = (struct elf_phdr*)&data[pht_offset+pht_entry_size*i];

        /* Skip if not loadable */
        if (entry->p_type != 1) continue;
        
        if (virtual_start == UINT64_MAX) virtual_start = entry->p_vaddr;
        if (virtual_end == 0) virtual_end = entry->p_vaddr;

        if (entry->p_vaddr < virtual_start) virtual_start = entry->p_vaddr;
        if (entry->p_vaddr > virtual_end) virtual_end = (entry->p_vaddr+entry->p_memsz-1);
    }

    for (elft_half i=0; i<pht_entry_count; i++) {
        struct elf_phdr* entry = (struct elf_phdr*)&data[pht_offset+pht_entry_size*i];

        /* Skip if not loadable */
        if (entry->p_type != 1) continue;
        if ((entry->p_filesz|entry->p_memsz|entry->p_align) > UINT32_MAX) {
            vga_print_color("Segments must be within 32 bit limit each\n", VGA_COLOR_RED);
            boot_panic();
            return meta;
        } else if (entry->p_vaddr-virtual_start > UINT32_MAX) {
            vga_print_color("Segment gap must be within 32 bits\n", VGA_COLOR_RED);
            boot_panic();
            return meta;
        }

        /* Permissions are not enforced, raw load is done */

        /* Data pointer in file */
        uint8_t* seg_start = (uint8_t*)&data[entry->p_offset];
        /* Virtual address pointer offsetted to load
         * relative to the address we gave */
        uint8_t* load_ptr = (uint8_t*)((uint32_t)address + (uint32_t)(entry->p_vaddr - virtual_start));

        /* Check alignment condition */
        if (((uint32_t)load_ptr & (entry->p_align-1)) != ((uint32_t)entry->p_offset & (entry->p_align-1))) {
            vga_print_color("Invalid segment alignment\n", VGA_COLOR_RED);
            boot_panic();
            return meta;
        }

        /* Copy segment from file as is */
        for (uint32_t b=0; b<(uint32_t)entry->p_filesz; b++) {
            load_ptr[b] = seg_start[b];
        }

        /* Fill memory image and physical image difference with 0s
         * if memsz > filesz */
        for (uint32_t b=entry->p_filesz; b<entry->p_memsz; b++) {
            load_ptr[b] = 0;
        }
    }

    /* Loaded all segments marked as load */ 

    meta.entrypoint = header->e_entry;
    meta.virtual_start = virtual_start;
    meta.memory_image_size = (uint32_t)virtual_end - (uint32_t)virtual_start;
    return meta;
}

