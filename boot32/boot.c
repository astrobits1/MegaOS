#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <boot32/vga.h>

void boot_main() {
	/* Initialize vga interface */
	vga_initialize();
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear();

	vga_print("Successfuly loaded boot32.efi in 32-bit protected mode\n");
    vga_print("Using VGA text buffer for logging\n");

    vga_print("0x");
    vga_print_u32(0x1FFFFFFF, 16, 8);
    vga_print("\n");
}

