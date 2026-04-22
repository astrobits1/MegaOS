#ifndef COMMON_GDT_H
#define COMMON_GDT_H

#include <stdint.h>
#include <stddef.h>

#define SEG_COUNT_BOOT 3
#define SEG_COUNT_KERNEL 5

#define SEG_SELECTOR_CODE 0x08
#define SEG_SELECTOR_DATA 0x10

/* boot32/gdt.c, kernel64/gdt.c */
void gdt_initialise();

/* boot32/gdt.c */
#ifdef __i686__
void gdt_set_long_mode();
#endif

/* boot32/descriptor.s, kernel64/descriptor.s */
void gdtr_load(void* offset, uint16_t size);
void reload_seg_regs();

#endif
