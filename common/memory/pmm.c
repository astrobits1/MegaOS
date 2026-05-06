#include <common/memory/pmm.h>
#include <common/bootinfo.h>
#include <stddef.h>

#include <common/drivers/vga/vga.h>

struct memory_map_entry* PMM_MEMORY_MAP = NULL;
uint32_t PMM_MEMORY_MAP_COUNT = 0;
uint64_t PMM_BOTTOM = 0;
uint64_t PMM_TOP = 0;

/* Free list head pointer for every order */
uint8_t* free_lists[18];

/* Initialize and mark top level zone */
void pmm_top_level_zone(uint64_t addr, uint8_t order) {
    vga_print("Top level: ");
    vga_print_uint_color(addr, 16, 16, VGA_COLOR_LIGHT_MAGENTA);
    vga_print(", Order: ");
    vga_print_uint_color(order, 10, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");
}

void pmm_allocate_page_list() {
    /* Read memory map and allocate page list spanning entire physical memory mapped
     * space */

    for (uint32_t i=0; i<PMM_MEMORY_MAP_COUNT; i++) {

    }
}

/* Load and initialize page list after reading usable zones */
void pmm_initialize_zones() {
    for (uint32_t i=0; i<PMM_MEMORY_MAP_COUNT; i++) {
        struct memory_map_entry entry = PMM_MEMORY_MAP[i];
        if (entry.addr >= PMM_TOP) break;
        if (entry.type != MEMORY_MAP_ENTRY_USABLE &&\
            entry.type != MEMORY_MAP_ENTRY_USABLE_ACPI)
            continue;

        uint64_t addr = entry.addr;
        uint64_t length = entry.length;

        /* Make sure page alignment is there */
        if (!CHECK_PAGE_4K_ALIGN(addr)) {
            uint64_t end = addr+length;
            addr = PAGE_4K_ALIGN(addr);
            length = end-addr;
        }
        /* There may be some wastage near the end of the mapping
         * if length is not page aligned, but will always be lesser than 4KB */
        if (!CHECK_PAGE_4K_ALIGN(length))
            length = PAGE_4K_ALIGN_DOWN(length);

        if (addr + length - 1 < PMM_BOTTOM)
            continue;

        if (addr < PMM_BOTTOM) {
            length = addr+length-PMM_BOTTOM;
            addr = PMM_BOTTOM;
        }

        if (addr + length - 1 >= PMM_TOP) 
            length -= addr+length - PMM_TOP;

        /* Length should end up at 0, as we have page aligned everything beforehand */
        while (length > 0) {
            /* Find highest order address alignment,
             * initialize block with highest possible order block given the size of region
             *
             * Order 0 is minimum requirement, which is ensured by previous page alignment */

            uint8_t highest_fit_od=18;
            for (int i=1; i<=18; i++) {
                if (!CHECK_ORDER_ALIGN(addr, i)) {
                    highest_fit_od = i-1;
                    break;
                }
            }

            uint8_t size_od = (64-__builtin_clzll(length))-(1+12);
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
    PMM_MEMORY_MAP = entries;
    PMM_MEMORY_MAP_COUNT = entry_count;
    PMM_BOTTOM = bottom;
    PMM_TOP = top;

    if (!CHECK_PAGE_4K_ALIGN(PMM_BOTTOM))
        PMM_BOTTOM = PAGE_4K_ALIGN(PMM_BOTTOM);
    if (!CHECK_PAGE_4K_ALIGN(PMM_TOP))
        PMM_TOP = PAGE_4K_ALIGN_DOWN(PMM_TOP);

    vga_print("PMM Bottom: "); vga_print_uint_color(PMM_BOTTOM, 16, 8, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\nPMM PMM_TOP: "); vga_print_uint_color(PMM_TOP, 16, 8, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");

    for (int i=0; i<18; i++) {
        free_lists[i] = NULL;
    }

    pmm_initialize_zones();
}

/* Allocate a 4KB page from physical memory, return NULL if not possible */
void* pmm_allocate_page() {
    if (PMM_MEMORY_MAP == NULL) return NULL;

        return NULL;
}
