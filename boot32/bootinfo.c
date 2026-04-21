#include <boot32/bootinfo.h>
#include <common/drivers/vga/vga.h>
#include <boot32/boot.h>
#include <boot32/allocator.h>


struct bootinfo parse_multiboot2_info(void* multiboot2_info) {
    uint32_t addr = (uint32_t)multiboot2_info;
    struct mbi_header* header = (void*)addr;
    uint32_t total_size = header->size;

    uint32_t startaddr = addr;
    addr += sizeof(struct mbi_header);

    /* Fields to read into from tags */
    struct bootinfo info;
    struct blob kernel_elf = {NULL, NULL, 0};
    struct memory_map_entry* mapentries = allocator_alloc_page();
    uint32_t memory_map_entry_count = 0;

    /* Start reading tags */
    while (addr-startaddr < total_size) {
        struct mbi_tag* tag = (void*)addr;

        switch (tag->type) {
            case MBI_TAG_MODULE: {
                struct mbi_tag_module* module = (void*)addr;
                
                kernel_elf.start = (void*)module->mod_start;
                kernel_elf.end = (void*)module->mod_end;
                kernel_elf.size = module->mod_end-module->mod_start;

                break;
            }
            case MBI_TAG_MEMORY_MAP: {
                struct mbi_tag_memory_map* map = (void*)addr;

                for (uint32_t offset=addr+16; (offset-addr) < tag->size; offset+=map->entry_size) {
                    struct mbi_memory_map_entry* mbi_entry = (void*)offset; 

                    if (memory_map_entry_count*sizeof(struct memory_map_entry) >= PAGE_4K) {
                        vga_print_color("Overflowed page while parsing memory map\n", VGA_COLOR_RED);
                        boot_panic();
                        return info;
                    }

                    struct memory_map_entry* entry = &mapentries[memory_map_entry_count];
                    entry->addr = mbi_entry->addr;
                    entry->length = mbi_entry->length;
                    
                    switch (mbi_entry->type) {
                        case 1: entry->type = MEMORY_MAP_ENTRY_USABLE; break;
                        case 3: entry->type = MEMORY_MAP_ENTRY_USABLE_ACPI; break;
                        case 4: entry->type = MEMORY_MAP_ENTRY_PRESERVE; break;
                        default: entry->type = MEMORY_MAP_ENTRY_UNUSABLE; break;
                    }

                    memory_map_entry_count++;
                }
                break;
            }
            default: break;
        }


        addr += tag->size;
        /* Next 8 byte aligned address after parsing tag */
        if ((addr & 0b111) != 0) {
            addr &= ~0b111;
            addr += 0b1000;
        }
    }

    /* Load kernel64.elf raw binary */

    if (kernel_elf.size == 0) {
        vga_print_color("Did not find kernel64.elf in multiboot2 modules\n", VGA_COLOR_RED);
        boot_panic();
        return info;
    } else info.kernel_elf = kernel_elf;

    /* Load memory map and count */

    info.map_entry_count = memory_map_entry_count;
    if (memory_map_entry_count == 0) {
        vga_print_color("Did not find memory map in multiboot2 info\n", VGA_COLOR_RED);
        boot_panic();
        return info;
    } else info.map_entries = mapentries;
    
    return info;
}

