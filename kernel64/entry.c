#include <kernel64/state.h>
#include <common/gdt.h>
#include <common/idt.h>
#include <common/bootinfo.h>
#include <common/drivers/vga/vga.h>

void kernel_setup() {
    /* Put the kernel in a stable long mode state */

    /* Set up kernel and user mode code and data segments + TSS (TODO)*/
    gdt_initialise();

    /* Initialise 64 bit IDT and link exception handlers */
    idt_initialise();
}

__attribute__((noreturn))
void kernel_main(struct bootinfo* info) {
    vga_initialize();
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BROWN);
    vga_clear();

    vga_print("Hello kernel64!\n");

    kernel_setup();
    vga_print("Done setup\n");

    lock();
}
