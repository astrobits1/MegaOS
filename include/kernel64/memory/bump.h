#ifndef KERNEL64_BUMP_H
#define KERNEL64_BUMP_H

#include <stdint.h>

int bump_initialize(uintptr_t bottom, uintptr_t top, uint64_t v_base);
void* bump_allocate_page();
void bump_free_page(void* page);
void* bump_get_physical(void* page);
void bump_uninitialize();

#endif
