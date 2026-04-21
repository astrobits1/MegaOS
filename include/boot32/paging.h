#ifndef BOOT32_PAGING_H
#define BOOT32_PAGING_H

#include <stdint.h>

#define M 52
/* 4KB */
#define MAP_SIZE PAGE_4K

#define PDE_2M_PAGE_MASK ~0x1FFFFF
#define PDPT_1G_PAGE_MASK ~0x3FFFFFFF
#define NO_PAGE_MASK 0
#define SET_PAGESIZE 1
#define NOSET_PAGESIZE 0

extern volatile void* PML4_0;
extern volatile void* PDPT_0;
extern volatile void* PD_0;

void paging_initialise();
void write_map_entry(volatile uint8_t* map, uint16_t index, uint64_t addr, uint8_t pagesize, uint32_t pagesize_mask);

/* paging_as.s */
void enable_paging64();
void load_pml4(volatile void* pml4);

#endif
