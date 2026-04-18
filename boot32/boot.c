#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <boot32/boot.h>
#include <boot32/vga.h>
#include <boot32/gdt.h>
#include <boot32/idt.h>

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
void boot_main(void* bootinfo) {
	/* Initialize vga interface */
	vga_initialize();
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear();

	vga_print_color("Successfuly loaded boot32.efi in 32-bit protected mode\n", VGA_COLOR_LIGHT_GREEN);
    vga_print_color("Using VGA text buffer for logging\n", VGA_COLOR_LIGHT_GREEN);

    vga_print("\n");
    vga_print("Boot Information struct address: 0x");
    vga_print_u32((uint32_t)bootinfo, 16, 8);
    vga_print("\n");

    /* Initialise global descriptor table (flat) */
    vga_print("\n");
    gdt_initialise();

    /* Initialise interrupt descriptor table, handle all faults
     * General Interrupt handlers are initialised later, and set to 
     * unimplemented handler for now */
    vga_print("\n");
    idt_initialise();


    lock();
}

