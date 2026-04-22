/* Stores all IRQ routines for faults, exceptions and interrupts */

#include <kernel64/state.h>
#include <kernel64/isr.h>
#include <common/isr_const.h>
#include <common/drivers/vga/vga.h>



/* Default exception handlers that just panic */

__attribute__((noreturn))
void exception_handler_panic_no_err(uint64_t n, struct interrupt_frame* frame) {
    if (n > 21) {n = 22;}

    vga_setfgcolor(VGA_COLOR_WHITE);
    vga_setbgcolor(VGA_COLOR_RED);

    vga_print("["); vga_print_uint(n, 10, -1); vga_print("] ");
    vga_print(exception_string[n]); vga_print("\n");

    vga_print("RIP: "); vga_print_uint(frame->rip, 16, 8); vga_print(", CS: "); 
    vga_print_uint(frame->cs, 16, 4);
    vga_print(", RFLAGS: "); vga_print_uint(frame->rflags, 16, -1); vga_print("\n");
    
    kernel_panic();
}

__attribute__((noreturn))
void exception_handler_panic_err(uint64_t n, uint64_t err, struct interrupt_frame* frame) {
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_RED);

    vga_print("ERROR: "); vga_print_uint(err, 16, -1); vga_print("\n");
    exception_handler_panic_no_err(n, frame);
}

/* For any other IRQs that arent implemented yet, also panic */

void interrupt_handler_unimplemented(uint64_t n, struct interrupt_frame* frame) {
    vga_setfgcolor(VGA_COLOR_BLACK);
    vga_print("Interrupt handler triggered, interrupt requested: 0x");
    vga_print_uint(n, 16, -1);
    vga_print("\n");
    vga_setfgcolor(VGA_COLOR_WHITE);
}
