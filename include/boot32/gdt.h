#ifndef BOOT32_GDT_H
#define BOOT32_GDT_H

#include <stdint.h>
#include <stddef.h>

#define SEG_COUNT_BOOT 3

#define SEG_SELECTOR_CODE 0x08
#define SEG_SELECTOR_DATA 0x10

/* boot32/gdt.c */
void gdt_initialise();

/* boot32/gdt.c */
void gdt_set_long_mode();

/* boot32/gdt_load.s */
void gdtr_load(void* offset, uint16_t size);
void reload_seg_regs();

#endif
