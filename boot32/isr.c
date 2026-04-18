/* Stores all IRQ routines for faults, exceptions and interrupts */

#include <boot32/boot.h>
#include <boot32/vga.h>

const char* exception_string[23] = {
    "Division error",
    "", "", "", "",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Co-Processor segment overrun",
    "Invalid task switch segment",
    "Segment not present",
    "Stack segmentation fault",
    "General protection fault",
    "Page fault",
    "Reserved CPU exception (illegal)",
    "FPU floating point fault",
    "Alignment error fault",
    "Machine check abort",
    "SIMD floating point exception",
    "Virtualization exception",
    "Control protection exception",

    /* 22-31 */
    "Reserved CPU exception"
};

/* All fault/abort exceptions cause kernel panic with no recovery */

__attribute__((noreturn))
void panic_exception_handler_no_err(uint32_t n, uint32_t eip, uint32_t cs, uint32_t eflags) {
    if (n > 21) {n = 22;}

    vga_setfgcolor(VGA_COLOR_WHITE);
    vga_setbgcolor(VGA_COLOR_RED);

    vga_print("["); vga_print_u32(n, 10, -1); vga_print("] ");
    vga_print(exception_string[n]); vga_print("\n");

    vga_print("EIP: "); vga_print_u32(eip, 16, 8); vga_print(", CS: "); vga_print_u32(cs, 16, 4);
    vga_print(", EFLAGS: "); vga_print_u32(eflags, 16, -1); vga_print("\n");
    
    boot_panic();
}

__attribute__((noreturn))
void panic_exception_handler_err(uint32_t n, uint32_t err, uint32_t eip, uint32_t cs, uint32_t eflags) {
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_RED);

    vga_print("ERROR: "); vga_print_u32(err, 16, -1); vga_print("\n");
    panic_exception_handler_no_err(n, eip, cs, eflags);
}

void unimplemented_routine() {
    vga_print("Unknown routine\n");

    boot_panic();
}
