#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <boot32/vga.h>

void boot_main(void) {
	/* Initialize vga interface */
	vga_initialize();

	vga_writestring("Hello, kernel World!\n");
    vga_writestring("Welcome to MegaOS\n");
}

