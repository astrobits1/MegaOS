#ifndef BOOT32_ALLOCATOR_H
#define BOOT32_ALLOCATOR_H

#include <stdint.h>

void allocator_initialise(void* bottom, void* top);
void* allocator_alloc_page();

#endif
