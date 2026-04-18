#ifndef BOOT32_GDT_H
#define BOOT32_GDT_H

#include <stdint.h>
#include <stddef.h>

#define SEG_DESCRIPTOR_ALLOC_COUNT 8
#define SEG_SELECTOR_CODE 0x08
#define SEG_SELECTOR_DATA 0x10

/* gdt.c */
void gdt_initialise();

/* descriptor.s */
void gdtr_load(void* offset, uint16_t size);
void reload_seg_regs();

#endif
