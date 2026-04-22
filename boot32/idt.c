#include <common/idt.h>
#include <common/gdt.h>
#include <common/drivers/vga/vga.h>
#include <boot32/isr.h>


#define IDT_ENTRY_SIZE32 8

enum GATE_TYPE {
    GATE_TASK = 0x5,
    GATE_INT_16 = 0x6,
    GATE_TRAP_16 = 0x7,
    GATE_INT_32 = 0xE,
    GATE_TRAP_32 = 0xF
};


uint16_t IDT[IDT_ENTRY_COUNT*IDT_ENTRY_SIZE32/2] __attribute__((aligned(16)));

void idt_write_entry(uint8_t index, uint32_t vec, enum GATE_TYPE gate_type) {
    uint32_t offset = index*IDT_ENTRY_SIZE32/2;
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
    //vga_print_color("Initializing Interrupt Descriptor Table\n", VGA_COLOR_LIGHT_GREEN);

    //vga_print("Initializing all entries to point to default unimplemented interrupt handler\n");
    for (int i=0; i<IDT_ENTRY_COUNT; i++) {
        idt_write_entry(i, (uint32_t)&unimplemented_routine, GATE_INT_32);
    }

    //vga_print("Setting exception handlers\n");

    for (int i=0; i<32; i++) {
        idt_write_entry(i, (uint32_t)isr_exception_table[i], GATE_INT_32);
    } 

    //vga_print("Loading IDT to IDTR\n");
    idtr_load((void*)IDT, IDT_ENTRY_COUNT*IDT_ENTRY_SIZE32-1);

    vga_print_color("Initialized Interrupt Descriptor Table\n", VGA_COLOR_LIGHT_GREEN);
}


