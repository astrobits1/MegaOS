#include <kernel64/memory/paging.h>
#include <common/drivers/vga/vga.h>

volatile void* PAGING_PML4 = NULL;
void* (*paging_allocate_page)() = NULL;
void (*paging_free_page)(void*) = NULL;
void* (*paging_get_physical)(void*) = NULL;

/* Initialize map to unmapped by default, (P bit 0) */
static void paging_initialize_map(volatile uint8_t* map) {
    for (uint32_t i=0; i<MAP_SIZE; i++) {
        map[i] = 0;
    }
}

/* Generalised function used to write address to pt, pd, pdpt, pml4
 *
 * pd and pdpt support pagesize bit which lets you define a contigious chunk of 2MB or 1GB 
 * in physical memory as a page. You write the physical memory address using SET_PAGESIZE and
 * the appropriate mask for them, otherwise pass NOSET_PAGESIZE, and NO_PAGE_MASK
 *
 * Flag configuration is minimal and default for now TODO */
static void paging_write_map_entry(volatile uint8_t* map, uint16_t index, uint64_t addr, uint8_t pagesize, uint64_t pagesize_mask) {
    uint64_t a = addr&(((uint64_t)1<<M)-1);

    uint32_t flag_data = 0x003;
    if (pagesize) {
        /* Interpret physical address as single 2MB/1GB page (only for PD/PDTE) */
        flag_data |= 1 << 7;
        a &= pagesize_mask;
    } else a &= ~0xFFF;

    a |= flag_data;
 
    uint64_t* ptr = (uint64_t*)&map[index*8];
    /*
    vga_print("Writing to: ");
    vga_print_uint((uintptr_t)ptr, 16, 16);
    vga_print("\n");
    */
    *ptr = a;
}

/* Read address in entry at index in map, and get pagesize info */
static uint64_t paging_read_map_entry(volatile uint8_t* map, uint16_t index, uint8_t* pagesize) { 
    uint64_t* ptr = (uint64_t*)&map[index*8];
    uint64_t a = ptr[0];

    *pagesize = a >> 7 & 1;
    /* Remove flag bits */
    a &= ~0xFFF;

    /* Address is M bits long */
    a &= ((uint64_t)1<<M)-1;
    return a;
}

/* Clear entry by setting present bit (and overall entry) to 0 */
static void paging_clear_map_entry(volatile uint8_t* map, uint16_t index) {
    uint64_t* ptr = (uint64_t*)&map[index*8];
    *ptr = 0;
}

/* Check if entry is present, by returning the present bit */
static bool paging_check_map_entry_present(volatile uint8_t* map, uint16_t index) {
    return (bool)(map[index*8] & 1);
}

/* Free map recursively, recursion is stopped when:
 * 1. We have gone 'depth' steps deep,
 * 2. or, when we have encountered an entry with PS=1 */
static void paging_free_map_recursive(volatile uint8_t* map, int depth) {
    for (uint16_t i=0; i<MAP_ENTRY_COUNT; i++) {
        if (!paging_check_map_entry_present(map, i)) continue;

        uint8_t PS;
        volatile void* addr = (volatile void*)(uintptr_t)paging_read_map_entry(map, i, &PS);
        if (PS == 1) continue;

        /* We have a nested map(s) which need to be freed */
        if (depth > 0)
            paging_free_map_recursive(addr, depth-1);

        paging_free_page((void*)addr);
    }
}

/* Initialize allocator, this is generalized to handle anything from basic
 * virtual memory at boot to full kernel management */
void paging_initialize_allocator(void* (*allocate_page)(), void (*free_page)(void*), void* (*get_physical)(void*)) {
    paging_allocate_page = allocate_page;
    paging_free_page = free_page;
    paging_get_physical = get_physical;
}

volatile void* paging_new_pml4() {
    volatile void* ptr = paging_allocate_page();
    if (ptr == NULL) return NULL;

    paging_initialize_map(ptr);
    return ptr;
}

void paging_free_pml4(volatile void* pml4) {
    paging_free_page((void*)pml4);
}

/* Set top level pml4 */
void paging_set_pml4(volatile void* pml4) {
    PAGING_PML4 = pml4;
}

void paging_reload_pml4(void(*reload)(volatile void* pml4)) {
    if (PAGING_PML4 == NULL) return;

    reload(PAGING_PML4);
}


/* Linear map virtual address to given physical address in PAGE_SIZE sized page blocks,
 * count represents no. of blocks starting from physical and mapped to virtual 1:1
 *
 * Virtual and Physical address must be page aligned to respective PAGE_SIZE or UND*/
int paging_map(uint64_t v_addr, uintptr_t p_addr, enum PAGE_SIZE size, uint32_t count) {
    /* Decode virtual address into PML4, PDPT, PD, PT indices
     * PT/PD and their entries may or may not be used based on size
     * If maps at those indices dont exist then create them */ 

    uint16_t pte = v_addr >> 12 & 0x1FF;
    uint16_t pde = v_addr >> 21 & 0x1FF;
    uint16_t pdpte = v_addr >> 30 & 0x1FF;
    uint16_t pml4e = (uint64_t)v_addr >> 39 & 0x1FF;

    volatile void* pt = NULL;
    volatile void* pd = NULL;
    volatile void* pdpt = NULL;
   
    for (uint32_t i=0; i<count; i++) {
        /* PML4E decoding */ 
        if (!paging_check_map_entry_present(PAGING_PML4, pml4e)) {
            volatile void* map = paging_allocate_page();
            if (map == NULL) 
                return 9;
            paging_initialize_map(map);

            paging_write_map_entry(PAGING_PML4, pml4e, (uintptr_t)paging_get_physical((void*)map), NOSET_PAGESIZE, NO_PAGE_MASK);
            pdpt = map;
            /* No free required as we just created a new one */
        } else {
            uint8_t PS;
            pdpt = (volatile void*)(uintptr_t)paging_read_map_entry(PAGING_PML4, pml4e, &PS);
        }

        /* PDPTE Write (For 1G pages) */
        if (size == PAGE_1G) {
            uint8_t prevPS;
            volatile void* prevMap = (volatile void*)(uintptr_t)paging_read_map_entry(\
                                        pdpt, pdpte, &prevPS);

            paging_write_map_entry(pdpt, pdpte, p_addr, SET_PAGESIZE, PDPT_1G_PAGE_MASK);

            if (prevMap != NULL && prevPS == 0) {
                /* A map exists here that has been unlinked after overwrite,
                 * free it recursively */
                paging_free_map_recursive(prevMap, PAGING_FREE_PDPTE_DEPTH);
            }

            p_addr += PAGE_1G;
            pdpte += 1;

            if ((p_addr + PAGE_1G) < p_addr) {
                vga_print_color("Physical address overflow while mapping\n", VGA_COLOR_RED);
                return 2;
            }

            goto pdpte_check;
        }

        /* PDPTE decoding (For 2M and 4K pages) */
        bool found = false;
        if (paging_check_map_entry_present(pdpt, pdpte)) {
            uint8_t PS;
            pd = (volatile void*)(uintptr_t)paging_read_map_entry(pdpt, pdpte, &PS);

            if (PS == 0) found = true;
        }

        if (!found) {
            volatile void* map = paging_allocate_page();
            if (map == NULL) 
                return 9;
            paging_initialize_map(map);

            paging_write_map_entry(pdpt, pdpte, (uintptr_t)paging_get_physical((void*)map), NOSET_PAGESIZE, NO_PAGE_MASK);
            pd = map;
        }

        /* PDE Write (For 2M pages) */
        if (size == PAGE_2M) {
            uint8_t prevPS;
            volatile void* prevMap = (volatile void*)(uintptr_t)paging_read_map_entry(\
                                        pd, pde, &prevPS);
            paging_write_map_entry(pd, pde, p_addr, SET_PAGESIZE, PDE_2M_PAGE_MASK);

            if (prevMap != NULL && prevPS == 0) {
                /* A map exists here that has been unlinked after overwrite,
                 * free it recursively */
                paging_free_map_recursive(prevMap, PAGING_FREE_PDE_DEPTH);
            }

            p_addr += PAGE_2M;
            pde += 1;

            if ((p_addr + PAGE_2M) < p_addr) {
                vga_print_color("Physical address overflow while mapping\n", VGA_COLOR_RED);
                return 2;
            }

            goto pde_check;
        }

        /* PDE decoding (For 4K pages) */
        found = false;
        if (paging_check_map_entry_present(pd, pde)) {
            uint8_t PS;
            pt = (volatile void*)(uintptr_t)paging_read_map_entry(pd, pde, &PS);

            if (PS == 0) found = true;
        }

        if (!found) {
            volatile void* map = paging_allocate_page();
            if (map == NULL) 
                return 9;
            paging_initialize_map(map);

            paging_write_map_entry(pd, pde, (uintptr_t)paging_get_physical((void*)map), NOSET_PAGESIZE, NO_PAGE_MASK);
            pt = map; 
        }
 
        /* PTE write (For 4K pages) */
        paging_write_map_entry(pt, pte, p_addr, NOSET_PAGESIZE, NO_PAGE_MASK); 

        //vga_print("1\n");
        if ((p_addr + PAGE_4K) < p_addr) {
            vga_print_color("Physical address overflow while mapping\n", VGA_COLOR_RED);
            return 2;
        }

        p_addr += PAGE_4K;
        pte += 1;
        /* Handle overflow into next maps */
        if (pte == MAP_ENTRY_COUNT) {
            pte = 0;
            pde += 1;
pde_check:
            if (pde == MAP_ENTRY_COUNT) {
                pde = 0;
                pdpte += 1;
pdpte_check:
                if (pdpte == MAP_ENTRY_COUNT) {
                    pdpte = 0;
                    pml4e += 1;
                    if (pml4e == MAP_ENTRY_COUNT) {
                        vga_print_color("Virtual address overflow while mapping\n", VGA_COLOR_RED);
                        return 3;
                    }
                }
            }
        }
    }

    return 0;
}

/* Unmaps 'count' number of PAGE_SIZE pages. 
 * Any underlying map structures which are unlinked are freed */
int paging_unmap(uint64_t v_addr, enum PAGE_SIZE size, uint32_t count) {
    /* Decode virtual address into PML4, PDPT, PD, PT indices
     * PT/PD and their entries may or may not be used based on size */
    uint16_t pte = v_addr >> 12 & 0x1FF;
    uint16_t pde = v_addr >> 21 & 0x1FF;
    uint16_t pdpte = v_addr >> 30 & 0x1FF;
    uint16_t pml4e = (uint64_t)v_addr >> 39 & 0x1FF;

    volatile void* pt = NULL;
    volatile void* pd = NULL;
    volatile void* pdpt = NULL;
    
    for (uint32_t i=0; i<count; i++) {
        /* PML4E decoding */
        if (!paging_check_map_entry_present(PAGING_PML4, pml4e))
            return 1;

        uint8_t PS;
        pdpt = (volatile void*)(uintptr_t)paging_read_map_entry(PAGING_PML4, pml4e, &PS);

        if (!paging_check_map_entry_present(pdpt, pdpte))
            return 2;

        pd = (volatile void*)(uintptr_t)paging_read_map_entry(pdpt, pdpte, &PS);
        if (size == PAGE_1G) {
            if (PS == 0) return 9;
            /* Clear entry */
            paging_clear_map_entry(pdpt, pdpte);
            pdpte += 1;
            goto pdpte_check;
        }

        if (!paging_check_map_entry_present(pd, pde))
            return 3;

        pt = (volatile void*)(uintptr_t)paging_read_map_entry(pd, pde, &PS);
        if (size == PAGE_2M) {
            if (PS == 0) return 8;
            /* Clear entry */
            paging_clear_map_entry(pd, pde);
            pde += 1;
            goto pde_check;
        }

        paging_clear_map_entry(pt, pte);
        pte += 1;
        /* Handle overflow into next maps */
        if (pte == MAP_ENTRY_COUNT) {
            pte = 0;
            pde += 1;
pde_check:
            if (pde == MAP_ENTRY_COUNT) {
                pde = 0;
                pdpte += 1;
pdpte_check:
                if (pdpte == MAP_ENTRY_COUNT) {
                    pdpte = 0;
                    pml4e += 1;
                    if (pml4e == MAP_ENTRY_COUNT) {
                        vga_print_color("Virtual address overflow while unmapping\n", VGA_COLOR_RED);
                        return 4;
                    }
                }
            }
        }
    }

    return 0;
}
