#ifndef BOOT32_GDT_H
#define BOOT32_GDT_H

#include <stdint.h>
#include <stddef.h>

/* gdt.c */
void gdt_initialise();

/* gdt_as.s */
void gdtr_load(void* offset, uint16_t size);
void reload_seg_regs();

#endif
