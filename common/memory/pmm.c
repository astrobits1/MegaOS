#include <common/memory/pmm.h>
#include <common/bootinfo.h>
#include <stddef.h>

struct memory_map_entry* MEMORY_MAP = NULL;
uint32_t MEMORY_MAP_COUNT = 0;
uint64_t BOTTOM = 0;
uint64_t TOP = 0;

/* Initialize and mark top level zone */
void pmm_top_level_zone(uint64_t addr, uint8_t order) {

}

void pmm_initialize_zones() {
    for (uint32_t i=0; i<MEMORY_MAP_COUNT; i++) {
        struct memory_map_entry entry = MEMORY_MAP[i];
        if (entry.addr >= TOP) break;
        if (entry.type != MEMORY_MAP_ENTRY_USABLE && entry.type != MEMORY_MAP_ENTRY_USABLE_ACPI)
            continue;

        uint64_t addr = entry.addr;
        uint64_t length = entry.length;

        /* Make sure page alignment is there */
        length -= addr&(PAGE_4K-1);
        addr = PAGE_4K_ALIGN(addr);
        /* There may be some wastage near the end of the mapping
         * if length is not page aligned, but will always be lesser than 4KB */
        length = PAGE_4K_ALIGN_DOWN(length);

        if (addr + length - 1 < BOTTOM)
            continue;

        if (addr < BOTTOM) {
            addr = BOTTOM;
            length -= BOTTOM - addr;
        }

        if (addr + length - 1 >= TOP) 
            length -= addr+length - TOP;

        /* Length should end up at 0, as we have page aligned everything beforehand */
        while (length > 0) {
            /* Find highest order address alignment,
             * initialize block with highest possible order block given the size of region
             *
             * Order 0 is minimum requirement, which is ensured by previous page alignment */
            uint8_t highest_fit_od=0;
            for (int i=0; i<=18; i++) {
                if (!CHECK_ORDER_ALIGN(addr, i)) {
                    highest_fit_od = i-1;
                    break;
                }
            }

            uint8_t size_od = (64-__builtin_clz(length))-(1+12);
            /* Cap at order 18 */
            if (size_od > 18) size_od = 18;

            /* Choose suitable order based on alignment and size constraints (min) */
            uint8_t order = highest_fit_od;
            if (order > size_od) order = size_od;
     
            pmm_top_level_zone(addr, order);

            uint64_t block_length = PAGE_4K<<order;
            length -= block_length;
            addr += block_length;
        }
    }
}

/* Get memory map in format specified in bootinfo.h,
 * get base physical address below which we dont touch memory */
void pmm_initialize(struct memory_map_entry* entries, uint32_t entry_count, uint64_t bottom, uint64_t top) {
    MEMORY_MAP = entries;
    MEMORY_MAP_COUNT = entry_count;
    BOTTOM = PAGE_4K_ALIGN(bottom);
    TOP = PAGE_4K_ALIGN(top);

    pmm_initialize_zones();
}

/* Allocate a 4KB page from physical memory, return NULL if not possible */
void* pmm_allocate_page() {
    if (MEMORY_MAP == NULL) return NULL;

        return NULL;
}
