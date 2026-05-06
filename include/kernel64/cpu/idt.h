#ifndef KERNEL64_IDT_H
#define KERNEL64_IDT_H

#include <stdint.h>

#define IDT_ENTRY_COUNT 256


/* kernel64/idt.c */
void idt_initialise();

/* kernel64/idy_load.s */
void idtr_load(void* offset, uint16_t size);

#endif
