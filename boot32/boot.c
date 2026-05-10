#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kernel64/memory/bump.h>
#include <kernel64/memory/paging.h>
#include <common/drivers/vga/vga.h>
#include <boot32/boot.h>
#include <boot32/gdt.h>
#include <boot32/idt.h>
#include <boot32/mbi2.h>
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

/* In paging_as.s */
extern void enable_paging64();
extern void load_pml4(volatile void* pml4);

void enable_paging() {
    /* Allocate page maps in physical memory, in a region that is outside .data
     * right at the end of the kernel 
     * They should be put at a page aligned physical address
     * Each map is 4KB in size 
     *
     * All the page structures should be within identity mapped region
     * or modifying them will cause a page fault */

    volatile void* pml4 = paging_new_pml4();
    paging_set_pml4(pml4);

    /* Identity map region from 0-(kernelend) to ensure a safe environment
     * after enabling paging TODO
     * Currently it maps full 2MB which will then cause a crash when remapped
     * by bump allocator */
    paging_map(0, 0, PAGE_2M, 2); 

    /* Load pml4 to CR3 */
    paging_reload_pml4(load_pml4);

    /* Enable 64 bit paging, and compatibility mode 
     * kernel and surrounding space should be identity mapped */
    enable_paging64();
    vga_print_color("Enabled 64 bit Paging and x86_64 compatibility mode\n", VGA_COLOR_LIGHT_GREEN);
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
    vga_print_uint_color((uint32_t)_bootstart, 16, 8, VGA_COLOR_LIGHT_GREEN);
    vga_print(", End Physical: ");
    vga_print_uint_color((uint32_t)_bootend, 16, 8, VGA_COLOR_LIGHT_GREEN);
    vga_print(", Size: ");
    vga_print_uint_color((uint32_t)(_bootend)-(uint32_t)(_bootstart), 10, -1, VGA_COLOR_LIGHT_GREEN);
    vga_print(" bytes\n\n");
    
    /* Initialise global descriptor table (flat) */
    gdt_initialise();

    /* Initialise interrupt descriptor table, handle all faults
     * General Interrupt handlers are set to 
     * unimplemented handler for now, interrupts are disabled */
    idt_initialise();

    /* Identity map is set up until 4MB and we do not expect boot kernel to occupy 
     * any more memory space beyond that point */
    uint32_t bottom = PAGE_4K_ALIGN((uint32_t)(_bootend));
    uint32_t top = 0x00400000;

    /* Parse boot info */
    vga_print("\n");
    vga_print_color("Parsing multiboot2 bootinfo\n", VGA_COLOR_LIGHT_GREEN);
    struct bootinfo info = parse_multiboot2_info(mb2_bootinfo, (void*)(bottom));
    bottom += info.map_entry_count*sizeof(struct memory_map_entry);

    if (!CHECK_PAGE_4K_ALIGN(bottom))
        bottom = PAGE_4K_ALIGN(bottom);

    vga_print_color("Found kernel64.elf\n", VGA_COLOR_LIGHT_GREEN); 
    bootinfo_print_memory_map(&info); 

    /* Enable SSE instructions */
    enable_sse();
    vga_print_color("SSE enabled\n", VGA_COLOR_LIGHT_GREEN);
    
    /* Initialise page maps, enable paging and enter x86_64 compatibility mode
     * 0-(kernel end) region is identity mapped, which includes IDT, GDT (data segments)
     */
    paging_initialize_allocator(bump_allocate_page, bump_free_page, bump_get_physical); 

    enable_paging();
    /* x86_64 compatibility mode has been entered, and our IDT is essentially invalid
     * until we enter long mode and set it up again */

    /* Initalize physical memory allocator, starting from bottom
     * (above the memory map allocation, at page boundary)
     * This sets up an identity map from (kernelend)-4MB, which 
     * is what all page allocations are confined to */
    bump_initialize(bottom, top, bottom);
    /* bump_allocate/free_page can now be used,
     * which means paging API can be used */

    /* Prepare kernel load by identity mapping 16 MB of memory starting at 4 MB */
    const uint8_t* kernel_physical_addr = (uint8_t*)0x00400000;
    const uint8_t initial_block_count = 8;
    paging_map((uint32_t)kernel_physical_addr, (uint32_t)kernel_physical_addr, PAGE_2M, initial_block_count);

    /* Load kernel ELF64 into identity mapped region and get entry point */ 
    struct elf_metadata meta = load_elf64_exec_at((void*)(uint32_t)info.kernel_elf.start, info.kernel_elf.size, (uint8_t*)kernel_physical_addr);

    vga_print("64 bit kernel loaded into identity mapped memory\n");
    vga_print("Virtual Address Start: ");
    vga_print_uint(meta.virtual_start>>32, 16, 8);
    vga_print_uint(meta.virtual_start&0xFFFFFFFF, 16, 8);
    vga_print("\n");
    vga_print("Entry point: ");
    vga_print_uint(meta.entrypoint>>32, 16, 8);
    vga_print_uint(meta.entrypoint&0xFFFFFFFF, 16, 8);
    vga_print("\n");
    vga_print("Memory image size: ");
    vga_print_uint(meta.memory_image_size, 10, -1);
    vga_print("\n");
   
    /* Map kernel memory to its expected 64 bit paging */
    paging_map(meta.virtual_start, (uint32_t)kernel_physical_addr, PAGE_2M, initial_block_count);
    vga_print_color("Kernel memory mapped to higher half 64 bit virtual address\n", VGA_COLOR_LIGHT_GREEN);
    /* Unmap temporary identity map */
    paging_unmap((uint32_t)kernel_physical_addr, PAGE_2M, initial_block_count); 

    /* Load kernel physical location data in bootinfo */
    info.kernel_physical.start = (uint32_t)kernel_physical_addr;
    info.kernel_physical.end = (uint32_t)kernel_physical_addr+meta.memory_image_size-1;
    info.kernel_physical.size = meta.memory_image_size;

    /* Make all entries 64 bit in GDT */
    vga_print_color("Setting up 64 bit GDT\n", VGA_COLOR_LIGHT_GREEN);
    gdt_set_long_mode();
    
    /* Long jump to entry point */
    vga_print_color("Ready to boot kernel in long mode, jumping...\n", VGA_COLOR_LIGHT_MAGENTA);

    jump_kernel(meta.entrypoint&0xFFFFFFFF, meta.entrypoint>>32, &info);
    /* noreturn */

    lock();
}

