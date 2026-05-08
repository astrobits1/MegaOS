#ifndef COMMON_PAGING_H
#define COMMON_PAGING_H

#include <stdint.h>
#include <stdbool.h>

#define M 52

enum PAGE_SIZE {
    PAGE_4K = 0x1000,
    PAGE_2M = 0x200000,
    PAGE_1G = 0x40000000
};

#define PAGE_4K_ALIGN(x) (((x)&~(PAGE_4K-1))+PAGE_4K)
#define PAGE_4K_ALIGN_DOWN(x) ((x)&~(PAGE_4K-1))

#define CHECK_PAGE_4K_ALIGN(x) (((x)&~(PAGE_4K-1))==(x))
#define CHECK_PAGE_2M_ALIGN(x) (((x)&~(PAGE_2M-1))==(x))
#define CHECK_PAGE_1G_ALIGN(x) (((x)&~(PAGE_1G-1))==(x))

#define MAP_SIZE PAGE_4K
#define MAP_ENTRY_COUNT 512

#define PDE_2M_PAGE_MASK ~0x1FFFFF
#define PDPT_1G_PAGE_MASK ~0x3FFFFFFF
#define NO_PAGE_MASK 0

#define SET_PAGESIZE 1
#define NOSET_PAGESIZE 0

/* Initialize page allocator */
void paging_initialize_allocator(void* (*allocate_page)(), void (*free_page)(void*));

/* Functions to work with PML4 (top level map entry) */
volatile void* paging_new_pml4();
void paging_free_pml4(volatile void* pml4);
void paging_set_pml4(volatile void* pml4);
/* Reload PML4 to CPU, which immediately takes action */
void paging_reload_pml4(void (*reload)(volatile void* pml4));

/* Functions to work with map entries */
void paging_initialize_map(volatile uint8_t* map);
void paging_write_map_entry(volatile uint8_t* map, uint16_t index, uint64_t addr, uint8_t pagesize, uint64_t pagesize_mask);
uint64_t paging_read_map_entry(volatile uint8_t* map, uint16_t index, uint8_t* pagesize);
bool paging_check_map_entry_present(volatile uint8_t* map, uint16_t index);

/* Functions to map physical to virtual */
int paging_map(uint64_t v_addr, uintptr_t p_addr, enum PAGE_SIZE size, uint8_t count);

#endif
