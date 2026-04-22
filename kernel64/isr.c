/* Stores all IRQ routines for faults, exceptions and interrupts */

#include <kernel64/state.h>
#include <common/isr.h>
#include <common/isr_const.h>
#include <common/drivers/vga/vga.h>

/* For kernel exceptions */
struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
};

/* For user mode exceptions, rsp and ss is pushed additionally */

struct interrupt_frame_user {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

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

void unimplemented_routine() {
    vga_print("Unknown routine\n");

    kernel_panic();
}
