#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <boot32/boot.h>
#include <boot32/vga.h>
#include <boot32/gdt.h>
#include <boot32/idt.h>
#include <boot32/paging.h>
#include <boot32/bootinfo.h>
#include <boot32/allocator.h>
#include <boot32/elf.h>

const void* _bootstart = (void*)&_BOOT_BEGIN;
const void* _bootend = (void*)&_BOOT_END;


__attribute__((noreturn))
void lock() {
l:
    __asm__ volatile("cli; hlt");
    goto l;
}

__attribute__((noreturn))
void boot_panic() {
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_print("Boot Kernel Panic!\nHalting\n");

    lock();
}

__attribute__((noreturn))
void boot_main(void* mb2_bootinfo) { 
	/* Initialize vga interface */
	vga_initialize();
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear();

	vga_print_color("Successfuly loaded boot32.efi in 32-bit protected mode\n", VGA_COLOR_LIGHT_GREEN);
    
    vga_print("\n");
    vga_print("Kernel Start Physical: ");
    vga_print_u32_color((uint32_t)_bootstart, 16, 8, VGA_COLOR_LIGHT_GREEN);
    vga_print(", End Physical: ");
    vga_print_u32_color((uint32_t)_bootend, 16, 8, VGA_COLOR_LIGHT_GREEN);
    vga_print(", Size: ");
    vga_print_u32_color((uint32_t)(_bootend)-(uint32_t)(_bootstart), 10, -1, VGA_COLOR_LIGHT_GREEN);
    vga_print(" bytes\n\n");
    
    /* Initialise global descriptor table (flat) */
    gdt_initialise();

    /* Initialise interrupt descriptor table, handle all faults
     * General Interrupt handlers are set to 
     * unimplemented handler for now, interrupts are disabled */
    idt_initialise();

    /* Initialise linear page allocator to closest 4KB page aligned physical address 
     * that doesnt overlap kernel
     * After the kernel image 
     *
     * Identity map is set up until 4MB and we do not expect boot kernel to occupy 
     * any more memory space beyond that point */
    void* heap_bottom = (void*)((uint32_t)(_bootend)&(~(PAGE_4K-1)))+PAGE_4K;
    void* heap_top = (void*)0x003FFFFF;
    allocator_initialise(heap_bottom, heap_top);

    /* Parse boot info */
    vga_print("\n");
    vga_print("Parsing multiboot2 bootinfo\n");
    struct bootinfo info = parse_multiboot2_info(mb2_bootinfo);

    vga_print_color("Found kernel64.elf\n", VGA_COLOR_LIGHT_GREEN);
    vga_print_color("Memory Map\n", VGA_COLOR_LIGHT_GREEN);
    for (uint32_t i=0; i<info.map_entry_count; i++) {
        struct memory_map_entry* entry = &info.map_entries[i];
        
        vga_print("base: "); vga_print_u32(entry->addr>>32, 16, 8); 
        vga_print_u32(entry->addr&0xFFFFFFFF, 16, 8);
        vga_print(", size: "); vga_print_u32(entry->length>>32, 16, 8); 
        vga_print_u32(entry->length&0xFFFFFFFF, 16, 8);
        vga_print(", type: "); vga_print_u32(entry->type, 10, -1); vga_print("\n");
    }

    vga_print("\n");

    /* Initialise page maps, enable paging and enter x86_64 compatibility mode
     * 0-4MB region is identity mapped, which includes IDT, GDT (data segments)
     * as well all page allocations by the allocator */
    paging_initialise();

    /* Prepare kernel load by identity mapping 16 MB of memory starting at 4 MB */
    const uint8_t* kernel_physical_addr = (uint8_t*)0x00400000;
    const uint8_t initial_block_count = 8;
    map_memory_2M((uint32_t)kernel_physical_addr, (uint32_t)kernel_physical_addr, initial_block_count);

    /* Load kernel ELF64 into identity mapped region and get entry point */ 
    
    struct elf_metadata meta = load_elf64_exec_at(info.kernel_elf.start, info.kernel_elf.size, (uint8_t*)kernel_physical_addr);

    vga_print("64 bit kernel loaded into identity mapped memory\n");
    vga_print("Virtual Address Start: ");
    vga_print_u32(meta.virtual_start>>32, 16, 8);
    vga_print_u32(meta.virtual_start&0xFFFFFFFF, 16, 8);
    vga_print("\n");
    vga_print("Entry point: ");
    vga_print_u32(meta.entrypoint>>32, 16, 8);
    vga_print_u32(meta.entrypoint&0xFFFFFFFF, 16, 8);
    vga_print("\n");
    vga_print("Memory image size: ");
    vga_print_u32(meta.memory_image_size, 10, -1);
    vga_print("\n");
    
    /* Enable SSE instructions */
    enable_sse();

    /* Map kernel memory to its expected 64 bit paging */
    map_memory_2M(meta.virtual_start, (uint32_t)kernel_physical_addr, initial_block_count);

    /* Make all entries 64 bit in GDT */
    gdt_set_long_mode();

    /* Long jump to entry point */
    //jump_kernel(meta);
    jump_kernel(meta.entrypoint&0xFFFFFFFF, meta.entrypoint>>32, &info);
    /* noreturn */

    lock();
}

