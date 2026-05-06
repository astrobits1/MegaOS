#ifndef COMMON_VMM_H
#define COMMON_VMM_H

#include <common/memory/pmm.h>
#include <stdint.h>
#include <stdbool.h>

#define M 52

#define MAP_SIZE PAGE_4K
#define MAP_ENTRY_COUNT 512

#define PDE_2M_PAGE_MASK ~0x1FFFFF
#define PDPT_1G_PAGE_MASK ~0x3FFFFFFF
#define NO_PAGE_MASK 0

#define SET_PAGESIZE 1
#define NOSET_PAGESIZE 0

extern volatile void* vmm_PML4;
void vmm_set_pml4(volatile void* pml4);

void vmm_initialise_map(volatile uint8_t* map);
void vmm_write_map_entry(volatile uint8_t* map, uint16_t index, uint64_t addr, uint8_t pagesize, uint64_t pagesize_mask);
uint64_t vmm_read_map_entry(volatile uint8_t* map, uint16_t index, uint8_t* pagesize);
bool vmm_check_map_entry_present(volatile uint8_t* map, uint16_t index);

int vmm_map_memory_2M(uintptr_t v_addr, uintptr_t p_addr, uint8_t count);

#endif
