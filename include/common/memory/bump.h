#ifndef COMMON_BUMP_H
#define COMMON_BUMP_H

#include <common/memory/memcommons.h>
#include <stdint.h>

void bump_initialize(uint64_t bottom, uint64_t top);
void* bump_allocate_page();
void bump_free_page(void* page);

#endif
