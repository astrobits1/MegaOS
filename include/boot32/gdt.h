#ifndef BOOT32_GDT_H
#define BOOT32_GDT_H

#include <stdint.h>
#include <stddef.h>

void gdt_initialise();
void gdtr_load(void* offset, uint16_t size);

#endif
