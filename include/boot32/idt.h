#ifndef BOOT32_IDT_H
#define BOOT32_IDT_H

#include <stdint.h>

/* idt.c */
void idt_initialise();

/* descriptor.s */
void idtr_load(void* offset, uint16_t size);

#endif
