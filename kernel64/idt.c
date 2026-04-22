#include <common/idt.h>
#include <common/gdt.h>
#include <common/drivers/vga/vga.h>
#include <common/isr.h>


#define IDT_ENTRY_SIZE64 16

enum GATE_TYPE {
    GATE_INT_64 = 0xE,
    GATE_TRAP_64 = 0xF
};

uint16_t IDT[IDT_ENTRY_COUNT*IDT_ENTRY_SIZE64/2];


void idt_write_entry(uint8_t index, uint64_t vec, enum GATE_TYPE gate_type) {
    uint32_t offset = index*IDT_ENTRY_SIZE64/2;
    /* Force ring 0 access for now 
     * as well as default code selector */
    uint8_t DPL = 0;
    uint8_t IST = 0;

    IDT[offset] = vec&0xFFFF;
    IDT[offset+1] = SEG_SELECTOR_CODE;
    IDT[offset+2] = (((gate_type&0xF)|(DPL<<5)|(1<<7)) << 8) | IST;
    IDT[offset+3] = vec>>16 & 0xFFFF;
    IDT[offset+4] = vec>>32 & 0xFFFF;
    IDT[offset+5] = vec>>48 & 0xFFFF;
    IDT[offset+6] = 0;
    IDT[offset+7] = 0;
}


void idt_initialise() {
    /* Set all service routine handlers to null */

    for (int i=0; i<IDT_ENTRY_COUNT; i++) {
        idt_write_entry(i, (uint64_t)&unimplemented_routine, GATE_INT_64);
    }

    for (int i=0; i<32; i++) {
        idt_write_entry(i, (uint64_t)isr_exception_table[i], GATE_INT_64);
    } 

    idtr_load((void*)IDT, IDT_ENTRY_COUNT*IDT_ENTRY_SIZE64-1);
}


