/* Stores all IRQ routines for faults, exceptions and interrupts */

#include <boot32/vga.h>

void irq0_division_error_fault() {
    vga_panic("Division Fault\n");

    for (;;);
}

void irq5_bound_range_exceed_fault() {
    vga_panic("Bound range exceeded\n");

    for (;;);
}

void irq6_invalid_opcode_fault() {
    vga_panic("Invalid Opcode\n");

    for (;;);
}

void irq7_dev_not_available_fault() {
    vga_panic("Device not available\n");

    for (;;);
}

void irq8_double_fault() {
    vga_panic("Double fault\n");

    for (;;);
}

void irq9_coprocessor_segment_overrun_fault() {
    vga_panic("Co-Processor segment overrun\n");

    for (;;);
}

void irq10_invalid_tss_fault() {
    vga_panic("Invalid Task Switch Segment\n");

    for (;;);
}

void irq11_segment_not_present_fault() {
    vga_panic("Segment not present\n");

    for (;;);

}

void irq12_stack_segment_fault() {
    vga_panic("Stack segmentation fault\n");

    for (;;);
}

void irq13_general_protection_fault() {
    vga_panic("General Protection Fault\n");
    
    for (;;);
}

void irq14_page_fault() {
    vga_panic("Page fault\n");

    for (;;);
}

void irq16_floating_point_fault() {
    vga_panic("x87 FPU Floating point fault\n");

    for (;;);
}

void irq17_alignment_check_fault() {
    vga_panic("Alignment error fault\n");

    for (;;);
}

void unimplemented_fault() {
    vga_panic("Unimplemented fault\n");

    for (;;);
}

void unimplemented_routine() {
    vga_print("Unknown routine\n");

    for (;;);
}
