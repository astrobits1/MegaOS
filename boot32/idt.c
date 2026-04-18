#include <boot32/idt.h>
#include <boot32/gdt.h>
#include <boot32/vga.h>
#include <boot32/irq_routines.h>

#define IDT_ENTRY_COUNT 256

volatile uint16_t IDT[IDT_ENTRY_COUNT*4];

enum GATE_TYPE {
    GATE_TASK = 0x5,
    GATE_INT_16 = 0x6,
    GATE_TRAP_16 = 0x7,
    GATE_INT_32 = 0xE,
    GATE_TRAP_32 = 0xF
};

void idt_write_entry(uint8_t index, uint32_t vec, enum GATE_TYPE gate_type) {
    uint32_t offset = index*4;
    /* Force ring 0 access for now 
     * as well as default code selector */
    uint8_t DPL = 0;

    IDT[offset] = vec&0xFFFF;
    IDT[offset+1] = SEG_SELECTOR_CODE;
    IDT[offset+2] = ((gate_type&0xF)|(DPL<<5)|(1<<7)) << 8;
    IDT[offset+3] = vec>>16 & 0xFFFF;
}


void idt_initialise() {
    /* Set all service routine handlers to null */
    vga_print_color("Initializing Interrupt Descriptor Table\n", VGA_COLOR_LIGHT_GREEN);

    vga_print("Initializing all entries to point to default unimplemented interrupt handler\n");
    for (int i=0; i<IDT_ENTRY_COUNT; i++) {
        idt_write_entry(i, (uint32_t)&unimplemented_routine, GATE_INT_32);
    }

    vga_print("Setting fault and abort handlers\n");
    idt_write_entry(0, (uint32_t)&irq0_division_error_fault, GATE_INT_32);
    idt_write_entry(5, (uint32_t)&irq5_bound_range_exceed_fault, GATE_INT_32);
    idt_write_entry(6, (uint32_t)&irq6_invalid_opcode_fault, GATE_INT_32);
    idt_write_entry(7, (uint32_t)&irq7_dev_not_available_fault, GATE_INT_32);
    idt_write_entry(8, (uint32_t)&irq8_double_fault, GATE_INT_32);
    idt_write_entry(9, (uint32_t)&irq9_coprocessor_segment_overrun_fault, GATE_INT_32);
    idt_write_entry(10, (uint32_t)&irq10_invalid_tss_fault, GATE_INT_32);
    idt_write_entry(11, (uint32_t)&irq11_segment_not_present_fault, GATE_INT_32);
    idt_write_entry(12, (uint32_t)&irq12_stack_segment_fault, GATE_INT_32);
    idt_write_entry(13, (uint32_t)&irq13_general_protection_fault, GATE_INT_32);
    idt_write_entry(14, (uint32_t)&irq14_page_fault, GATE_INT_32);
    idt_write_entry(16, (uint32_t)&irq16_floating_point_fault, GATE_INT_32);
    idt_write_entry(17, (uint32_t)&irq17_alignment_check_fault, GATE_INT_32);
   
    /* Miscalleneous faults */
    for (int i=18; i<22; i++) idt_write_entry(i, (uint32_t)&unimplemented_fault, GATE_INT_32);

    vga_print("Loading IDT to IDTR\n");
    idtr_load((void*)IDT, IDT_ENTRY_COUNT*8-1);

    vga_print_color("Initialized Interrupt Descriptor Table\n", VGA_COLOR_LIGHT_GREEN);
}


