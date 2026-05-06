#ifndef KERNEL64_GDT_H
#define KERNEL64_GDT_H

#include <stdint.h>
#include <stddef.h>

#define SEG_COUNT_KERNEL 5

#define SEG_SELECTOR_CODE 0x08
#define SEG_SELECTOR_DATA 0x10

/* kernel64/gdt.c */
void gdt_initialise();

/* kernel64/gdt_load.s */
void gdtr_load(void* offset, uint16_t size);
void reload_seg_regs();

#endif
