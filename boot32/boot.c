#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <boot32/vga.h>
#include <boot32/gdt.h>

void boot_main(void* bootinfo) {
	/* Initialize vga interface */
	vga_initialize();
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear();

	vga_print("Successfuly loaded boot32.efi in 32-bit protected mode\n");
    vga_print("Using VGA text buffer for logging\n");

    vga_print("\n");
    vga_print("Boot Information struct address: 0x");
    vga_print_u32((uint32_t)bootinfo, 16, 8);
    vga_print("\n");

    /* Initialise global descriptor table (flat) */
    gdt_initialise();
}

