#ifndef COMMON_IDT_H
#define COMMON_IDT_H

#include <stdint.h>

#define IDT_ENTRY_COUNT 256


/* boot32/idt.c kernel64/idt.c */
void idt_initialise();

/* boot32/idt_load.s kernel64/idy_load.s */
void idtr_load(void* offset, uint16_t size);

#endif
