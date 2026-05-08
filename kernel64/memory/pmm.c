#include <kernel64/memory/paging.h>
#include <kernel64/memory/pmm.h>
#include <common/bootinfo.h>
#include <stddef.h>

#include <common/drivers/vga/vga.h>

struct memory_map_entry* PMM_MEMORY_MAP = NULL;
uint32_t PMM_MEMORY_MAP_COUNT = 0;
uint64_t PMM_BOTTOM = 0;
uint64_t PMM_TOP = 0;

uint64_t PMM_PAGE_LIST_TOP = 0;
uint64_t PMM_PAGE_LIST_BOTTOM = 0;

uint64_t PMM_ZONE_LIST_BOTTOM = 0;
uint32_t PMM_ZONE_COUNT = 0;

/* Free list head pointer for every order */
uint8_t* free_lists[18];

/* Initialize and mark top level zone */
static void pmm_top_level_zone(uint64_t addr, uint8_t order) {
    vga_print("Top level: ");
    vga_print_uint_color(addr, 16, 16, VGA_COLOR_LIGHT_MAGENTA);
    vga_print(", Order: ");
    vga_print_uint_color(order, 10, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");
}

/* Raw write to page metadata, no guards or checks */
static void pmm_page_meta_write(struct pmm_page_meta* list_base, uint64_t page_index, uint8_t state, uint8_t order) {
    struct pmm_page_meta* meta = &list_base[page_index];
    meta->order = order;
    meta->state = state;
}

static int pmm_allocate_lists(uint64_t base, uint32_t zone_count, uint64_t list_size) {
    /* Read memory map and allocate page list and zone list starting at
     * the base address, size has been precalculated and it is ensured that
     * the memory we are writing to is free and usable */
    
    PMM_ZONE_LIST_BOTTOM = base;
    PMM_PAGE_LIST_BOTTOM = sizeof(struct pmm_zone_meta)*zone_count;
    PMM_PAGE_LIST_TOP = base+list_size;
    PMM_ZONE_COUNT = zone_count;

    vga_print("Allocating lists now\n");
    vga_print("List Size Calculated: ");
    vga_print_uint(list_size, 10, -1);
    vga_print("\nZone Count: ");
    vga_print_uint(zone_count, 10, -1);
    vga_print("\nAllocating at: ");
    vga_print_uint(base, 16, 12);
    vga_print("\n");
    vga_print("Mapping physical to virtual identity starting at base,\nfor ");
    vga_print_uint(((list_size-1)>>12)+1, 10, -1);
    vga_print(" 4K pages\n");

    /* In order to access physical memory, we have to page it into an accessible
     * virtual range, as we are mapped and operating in virtual memory
     * For now, we can just identity map for easy translation TODO */
    if (paging_map(base, base, PAGE_4K, ((list_size-1)>>12)+1))
        return 1;

    uint64_t page_list_base = PMM_PAGE_LIST_BOTTOM;
    for (uint32_t i=0; i<PMM_MEMORY_MAP_COUNT; i++) {
        struct memory_map_entry entry = PMM_MEMORY_MAP[i];
        bool usable = true;

        if (entry.type != MEMORY_MAP_ENTRY_USABLE &&\
            entry.type != MEMORY_MAP_ENTRY_USABLE_ACPI)
            usable = false;

        /* A zone is any usable RAM region
         * Write zone list entry */
        struct pmm_zone_meta* zone_meta = (struct pmm_zone_meta*)base; 
        zone_meta->usable = usable;

        if (!usable) {
            zone_meta->base = 0;
            zone_meta->length = 0;
            zone_meta->page_count = 0;
            base += sizeof(struct pmm_zone_meta);
            continue;
        }

        uint64_t addr, length;
        if (!CHECK_PAGE_4K_ALIGN(entry.addr))
            addr = PAGE_4K_ALIGN(entry.addr);
        else
            addr = entry.addr;

        if (!CHECK_PAGE_4K_ALIGN(entry.length))
            length = PAGE_4K_ALIGN_DOWN(entry.length);
        else
            length = entry.length;

        zone_meta->page_count = ((length-1)>>12)+1;
        zone_meta->base = page_list_base;
        zone_meta->length = zone_meta->page_count*sizeof(struct pmm_page_meta);

        /* For next zone, if any */
        base += sizeof(struct pmm_zone_meta);
        page_list_base += zone_meta->base + zone_meta->length;

        /* Write page entry list at corresponding offset
         * Initialize to free and order 0 for now */
        for (uint64_t page_meta_i=0; page_meta_i<zone_meta->page_count; page_meta_i++) {
            pmm_page_meta_write((struct pmm_page_meta*)zone_meta->base, page_meta_i, PMM_PAGE_FREE, 0);
        }
    }
    
    return 0;
}

/* Calculate page list + zone list size */
static uint64_t pmm_get_list_size(uint32_t* _zone_count) {
    uint64_t list_size = 0;
    uint32_t zone_count = 0;
    for (uint32_t i=0; i<PMM_MEMORY_MAP_COUNT; i++) {
        struct memory_map_entry entry = PMM_MEMORY_MAP[i];
        if (entry.type != MEMORY_MAP_ENTRY_USABLE &&\
            entry.type != MEMORY_MAP_ENTRY_USABLE_ACPI)
            continue;

        uint64_t addr = entry.addr;
        uint64_t length = entry.length;

        if (!CHECK_PAGE_4K_ALIGN(addr))
            addr = PAGE_4K_ALIGN(addr);
        if (!CHECK_PAGE_4K_ALIGN(length))
            length = PAGE_4K_ALIGN_DOWN(length);

        uint64_t max_local_index = (length-1)>>12;
        uint64_t local_list_size = (max_local_index+1)*sizeof(struct pmm_page_meta);
        uint64_t local_size = sizeof(struct pmm_zone_meta) + local_list_size;

        list_size += local_size;
        zone_count++;
    }

    *_zone_count = zone_count;

    /* Align list size to page boundary */
    if (!CHECK_PAGE_4K_ALIGN(list_size))
        list_size = PAGE_4K_ALIGN(list_size);
    return list_size;
}

/* Load and initialize page list after reading usable zones */
static int pmm_initialize_zones() {
    uint32_t zone_count = 0;
    uint64_t list_size = pmm_get_list_size(&zone_count);

    bool list_allocated = false;

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

        /* Handle the case of allocating page list inside
         * the free ram zone somewhere, we cut the zone early
         * and resume by jumping back to the loop after skipping
         * the region */
        if (!list_allocated && length >= list_size) {
            int s = pmm_allocate_lists(addr, zone_count, list_size);

            /* Error occured during allocation */
            if (s) 
                return s;
            addr = PMM_PAGE_LIST_TOP;
            length -= list_size;
            vga_print("Successfully allocated lists\n");
            list_allocated = true;
        }
                
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
 
            //pmm_top_level_zone(addr, order);

            uint64_t block_length = PAGE_4K<<order;

            length -= block_length;
            addr += block_length; 
        }
    }

    /* Suitable region couldnt be found to allocate lists */
    if (!list_allocated)
        return 2;

    return 0;
}

/* Get memory map in format specified in bootinfo.h,
 * get base physical address below which we dont touch memory */
int pmm_initialize(struct memory_map_entry* entries, uint32_t entry_count, uint64_t bottom, uint64_t top) {
    PMM_MEMORY_MAP = entries;
    PMM_MEMORY_MAP_COUNT = entry_count;
    PMM_BOTTOM = bottom;
    PMM_TOP = top;

    if (!CHECK_PAGE_4K_ALIGN(PMM_BOTTOM))
        PMM_BOTTOM = PAGE_4K_ALIGN(PMM_BOTTOM);
    if (!CHECK_PAGE_4K_ALIGN(PMM_TOP))
        PMM_TOP = PAGE_4K_ALIGN_DOWN(PMM_TOP);

    vga_print("PMM Bottom: "); vga_print_uint_color(PMM_BOTTOM, 16, 8, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\nPMM Top: "); vga_print_uint_color(PMM_TOP, 16, 8, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");

    for (int i=0; i<18; i++) {
        free_lists[i] = NULL;
    }

    int s = pmm_initialize_zones();
    if (s)
        return s;
    
    return 0;
}

/* Allocate a 4KB page from physical memory, return NULL if not possible */
void* pmm_allocate_page() {
    if (PMM_MEMORY_MAP == NULL) return NULL;

    return NULL;
}
