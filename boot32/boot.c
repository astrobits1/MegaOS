#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <boot32/boot.h>
#include <boot32/vga.h>
#include <boot32/gdt.h>
#include <boot32/idt.h>
#include <boot32/paging.h>
#include <boot32/bootinfo.h>

const void* _bootstart = (void*)&_BOOT_BEGIN;
const void* _bootend = (void*)&_BOOT_END;


__attribute__((noreturn))
void lock() {
l:
    __asm__ volatile ("cli; hlt");
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
    vga_print_color("Using VGA text buffer for logging\n", VGA_COLOR_LIGHT_GREEN);
    
    vga_print("\n");
    vga_print("Kernel Start Physical: ");
    vga_print_u32_color((uint32_t)_bootstart, 16, 8, VGA_COLOR_LIGHT_GREEN);
    vga_print(", End Physical: ");
    vga_print_u32_color((uint32_t)_bootend, 16, 8, VGA_COLOR_LIGHT_GREEN);
    vga_print(", Size: ");
    vga_print_u32_color((uint32_t)(_bootend)-(uint32_t)(_bootstart), 10, -1, VGA_COLOR_LIGHT_GREEN);
    vga_print(" bytes\n\n");
    
    /*
    vga_print("\n");
    vga_print("Boot Information struct address: 0x");
    vga_print_u32((uint32_t)bootinfo, 16, 8);
    vga_print("\n");
    */
    /* Initialise global descriptor table (flat) */
    gdt_initialise();

    /* Initialise interrupt descriptor table, handle all faults
     * General Interrupt handlers are initialised later, and set to 
     * unimplemented handler for now */
    idt_initialise();

    /* Parse boot info */
    vga_print("\n");
    vga_print("Parsing multiboot2 bootinfo\n");
    struct bootinfo info = parse_multiboot2_info(mb2_bootinfo);

    if (info.kernel_elf.size == 0) {
        vga_print("kernel64.elf not found\n");
    } else {
        vga_print("kernel64.elf found\n");
    }

    vga_print("\n");

    /* Initialise page maps, enable paging and enter x86_64 compatibility mode */
    paging_initialise();

    /* Load kernel ELF64 */
    
    lock();
}

