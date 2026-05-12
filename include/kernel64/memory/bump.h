#ifndef KERNEL64_BUMP_H
#define KERNEL64_BUMP_H

#include <stdint.h>

int bump_initialize(uintptr_t bottom, uintptr_t top, uint64_t v_base);
void bump_uninitialize();

void* bump_allocate_page();
void bump_free_page(void* page);

uintptr_t bump_p_ptr(void* v_ptr);
void* bump_v_ptr(uintptr_t p_ptr);

#endif
