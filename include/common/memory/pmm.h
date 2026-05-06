#ifndef COMMON_PMM_H
#define COMMON_PMM_H

#include <common/bootinfo.h>
#include <stdint.h>

#define PAGE_4K 0x1000
#define PAGE_2M 0x200000

#define PAGE_4K_ALIGN(x) (((x)&~(PAGE_4K-1))+PAGE_4K)
#define PAGE_4K_ALIGN_DOWN(x) ((x)&~(PAGE_4K-1))

#define CHECK_PAGE_4K_ALIGN(x) (((x)&~(PAGE_4K-1))==(x))
/* 4KB is order 0 (fundamental page)
 * 8KB is order 1
 * ...
 * 1GB is order 18 (max supported order)
 *
 * This macro checks if given memory address is aligned to the given order block size */
#define CHECK_ORDER_ALIGN(x, od) (((x)&~(((uint64_t)PAGE_4K<<(od))-1))==(x))

void pmm_initialize(struct memory_map_entry* entries, uint32_t entry_count, uint64_t bottom, uint64_t top);
void* pmm_allocate_page();

#endif
