#ifndef BOOT32_PAGING_H
#define BOOT32_PAGING_H

#include <stdint.h>

#define M 52

/* 4KB */
#define MAP_SIZE PAGE_4K
#define MAP_ENTRY_COUNT 512

#define PDE_2M_PAGE_MASK ~0x1FFFFF
#define PDPT_1G_PAGE_MASK ~0x3FFFFFFF
#define NO_PAGE_MASK 0
#define SET_PAGESIZE 1
#define NOSET_PAGESIZE 0

extern volatile void* PML4;

void paging_initialise();
void write_map_entry(volatile uint8_t* map, uint16_t index, uint32_t addr, uint8_t pagesize, uint32_t pagesize_mask);
uint32_t read_map_entry(volatile uint8_t* map, uint16_t index, uint8_t* pagesize);
void map_memory_2M(uint64_t v_addr, uint32_t p_addr, uint8_t count);

/* paging_as.s */
void enable_paging64();
void load_pml4(volatile void* pml4);

#endif
