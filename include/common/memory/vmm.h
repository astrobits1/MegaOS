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

/* Initialize page allocator */
void vmm_initialize_allocator(void* (*allocate_page)(), void (*free_page)(void*));

/* Functions to work with PML4 (top level map entry) */
volatile void* vmm_new_pml4();
void vmm_free_pml4(volatile void* pml4);
void vmm_set_pml4(volatile void* pml4);
/* Reload PML4 to CPU, which immediately takes action */
void vmm_reload_pml4(void (*reload)(volatile void* pml4));

/* Functions to work with map entries */
void vmm_initialize_map(volatile uint8_t* map);
void vmm_write_map_entry(volatile uint8_t* map, uint16_t index, uint64_t addr, uint8_t pagesize, uint64_t pagesize_mask);
uint64_t vmm_read_map_entry(volatile uint8_t* map, uint16_t index, uint8_t* pagesize);
bool vmm_check_map_entry_present(volatile uint8_t* map, uint16_t index);

/* Functions to map physical to virtual */
int vmm_map_memory_2M(uint64_t v_addr, uintptr_t p_addr, uint8_t count);

#endif
