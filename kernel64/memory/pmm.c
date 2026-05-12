#include <kernel64/memory/paging.h>
#include <kernel64/memory/pmm.h>
#include <common/bootinfo.h>
#include <stddef.h>

#include <common/drivers/vga/vga.h>

/* Important conventions:
 *
 * Physical addresses are always unsigned integers specified by uintptr_t
 * and additionally have prefix p_ before name
 *
 * Virtual addresses should preferably be used and stored only as pointers
 * with no prefix, and when converting to integer, use uint64_t
 *
 * Physical addresses must use 0 as 'NULL' and virtual addresses must use NULL,
 * interconverting them should yield the other
 *
 * Physical addresses are what we reason with in the algorithm,
 * while virtual addresses are how we communicate with other subsystems, internally,
 * or for reads/writes. 
 *
 * Essentially virtual addressing here is just the physical address range
 * mirrored into a virtual range with a known base, and offsets are preserved.
 * Use pmm_v_ptr/pmm_p_ptr to convert between them
 *
 * Internally functions can expect either based on whichever is convenient */

struct pmm_pvt {
    struct memory_map_entry* memory_map;
    uint32_t memory_map_count;
    uintptr_t p_bottom;
    uintptr_t p_top;

    /* Virtual address base pointer where usable PMM owned RAM is mirrored 
     * with physical offsets */
    uint64_t mirror_base;

    struct pmm_page_meta* page_list_top;
    struct pmm_page_meta* page_list_bottom;

    struct pmm_zone_meta* zone_list_bottom;
    uint32_t zone_usable_count;
    /* Free list head pointer for every order */
    uint8_t* free_lists[18];
};

struct pmm_pvt pmm_state;

/* Initialize and mark top level block */
static void pmm_top_level_block(uint64_t addr, uint8_t order) {
    /*
    vga_print("Top level: ");
    vga_print_uint_color(addr, 16, 16, VGA_COLOR_LIGHT_MAGENTA);
    vga_print(", Order: ");
    vga_print_uint_color(order, 10, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");
    */
}

/* Raw read from metadata array */
static inline struct pmm_page_meta* pmm_page_meta_read(struct pmm_page_meta* list_base, uint64_t page_index) {
    return &list_base[page_index];
}

/* Raw write to page metadata, no guards or checks */
static void pmm_page_meta_write(struct pmm_page_meta* list_base, uint64_t page_index, uint8_t state, uint8_t order) {
    struct pmm_page_meta* meta = pmm_page_meta_read(list_base, page_index);
    meta->order = order;
    meta->state = state;
}

/* Print metadata */
static void pmm_page_meta_print(struct pmm_page_meta* meta) {
    vga_print("State: ");
    switch (meta->state) {
        case PMM_PAGE_ALLOCATED:
            vga_print_color("ALLOCATED", VGA_COLOR_LIGHT_MAGENTA);
            break;
        case PMM_PAGE_FREE:
            vga_print_color("FREE", VGA_COLOR_LIGHT_MAGENTA);
            break;
        case PMM_PAGE_RESERVED:
            vga_print_color("RESERVED", VGA_COLOR_LIGHT_MAGENTA);
            break;
        default:
            vga_print_color("UNKNOWN", VGA_COLOR_LIGHT_RED);
            break;
    }
    vga_print(" | Order: ");
    vga_print_uint_color(meta->order, 10, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");
}

/* Given a physical page address, look up its zone if its usable
 * otherwise return NULL */
static struct pmm_zone_meta* pmm_zone_meta_linear_lookup(uintptr_t p_addr) {
    for (uint32_t i=0; i<pmm_state.memory_map_count; i++) {
        struct memory_map_entry entry = pmm_state.memory_map[i];

        if (p_addr >= entry.addr && p_addr < entry.addr+entry.length) {
            /* Get corresponding zone metadata once memory map region
             * has been identified */
            struct pmm_zone_meta* base = pmm_state.zone_list_bottom;
            struct pmm_zone_meta* meta = &base[i];

            if (!meta->usable) 
                return NULL;

            return meta;
        }
    }

    /* It is unmapped */
    return NULL;
}

static struct pmm_page_meta* pmm_page_meta_linear_lookup_read(uintptr_t p_addr) {
    struct pmm_zone_meta* zone_meta = pmm_zone_meta_linear_lookup(p_addr);
    /* Check for zone validity */
    if (zone_meta == NULL) 
        return NULL;

    /* Within zone, find page index */
    uint64_t page_index = (p_addr-zone_meta->zone_p_base)>>12;

    /* Lookup page metadata and return it */
    struct pmm_page_meta* page_meta = pmm_page_meta_read(zone_meta->page_list_base, page_index);
    return page_meta;
}

static int pmm_page_meta_linear_lookup_write(uintptr_t p_addr, uint8_t state, uint8_t order) {
    /* Lookup page metadata and write to it */
    struct pmm_page_meta* page_meta = pmm_page_meta_linear_lookup_read(p_addr);
    if (page_meta == NULL)
        return 1;
    page_meta->order = order;
    page_meta->state = state;

    return 0;
}

static int pmm_allocate_lists(uintptr_t p_zone_list_base, uint32_t zone_count, uint64_t list_size) {
    /* Read memory map and allocate page list and zone list starting at
     * the base address, size has been precalculated and it is ensured that
     * the memory we are writing to is free and usable */
   
    /* In order to access physical memory, we have to page it into an accessible
     * virtual range, as we are mapped and operating in virtual memory
     * PMM has to have set a mirror map at least for the zone we are allocating this
     * page list in 
     *
     * Any pointers we use for internal use are always virtual addresses 
     * we can directly access and write. We can convert back and forth between
     * virtual and physical because of the mirroring scheme */


    uintptr_t p_page_list_base = p_zone_list_base+sizeof(struct pmm_zone_meta)*pmm_state.memory_map_count;
    pmm_state.zone_list_bottom = pmm_v_ptr(p_zone_list_base);
    pmm_state.page_list_bottom = pmm_v_ptr(p_page_list_base);
    pmm_state.page_list_top = pmm_v_ptr(p_zone_list_base+list_size);
    pmm_state.zone_usable_count = zone_count; 
    
    for (uint32_t i=0; i<pmm_state.memory_map_count; i++) {
        struct memory_map_entry entry = pmm_state.memory_map[i];
        bool usable = true;

        if (entry.type != MEMORY_MAP_ENTRY_USABLE &&\
            entry.type != MEMORY_MAP_ENTRY_USABLE_ACPI)
            usable = false;

        /* A zone is any usable RAM region
         * Write zone list entry */
        struct pmm_zone_meta* zone_meta = pmm_v_ptr(p_zone_list_base); 
        uintptr_t zone_base_addr;
        uint64_t zone_length;

        if (!CHECK_PAGE_4K_ALIGN(entry.length))
            zone_length = PAGE_4K_ALIGN_DOWN(entry.length);
        else
            zone_length = entry.length;

        if (!CHECK_PAGE_4K_ALIGN(entry.addr))
            zone_base_addr = PAGE_4K_ALIGN_DOWN(entry.addr);
        else
            zone_base_addr = entry.addr;

        zone_meta->usable = usable;
        zone_meta->zone_p_base = zone_base_addr;

        if (!usable) {
            zone_meta->page_list_base = NULL;
            zone_meta->page_list_length = 0;
            zone_meta->page_count = 0;
            p_zone_list_base += sizeof(struct pmm_zone_meta);
            continue;
        }

        zone_meta->page_count = PAGE_COUNT_4K_CONTAINING_LEN(zone_length);
        zone_meta->page_list_base = pmm_v_ptr(p_page_list_base);
        zone_meta->page_list_length = zone_meta->page_count*sizeof(struct pmm_page_meta);

        /* For next zone, if any */
        p_zone_list_base += sizeof(struct pmm_zone_meta);
        p_page_list_base += zone_meta->page_list_length;

        /* Write page entry list at corresponding offset
         * Initialize to free and order 0 for now */
        for (uint64_t page_meta_i=0; page_meta_i<zone_meta->page_count; page_meta_i++) {
            pmm_page_meta_write(zone_meta->page_list_base, page_meta_i, PMM_PAGE_FREE, 0);
        }
    }
 
    /* Zone and page lists have been written and initialized,
     * we can now mark the pages occupied by the lists themselves as RESERVED */ 
    struct pmm_zone_meta* reserved_zone_meta = pmm_zone_meta_linear_lookup(\
            pmm_p_ptr(pmm_state.zone_list_bottom));
    uint64_t base_page_index = PAGE_COUNT_4K_CONTAINING_LEN( \
            pmm_p_ptr(pmm_state.zone_list_bottom)-reserved_zone_meta->zone_p_base);

    for (uint64_t i=0; i<PAGE_COUNT_4K_CONTAINING_LEN(list_size); i++) {
        pmm_page_meta_write(reserved_zone_meta->page_list_base, \
                base_page_index+i, PMM_PAGE_RESERVED, 0);
    }

    return 0;
}

/* Calculate page list + zone list size */
static uint64_t pmm_get_list_size(uint32_t* _zone_count) {
    uint64_t list_size = 0;
    uint32_t zone_count = 0;
    for (uint32_t i=0; i<pmm_state.memory_map_count; i++) {
        struct memory_map_entry entry = pmm_state.memory_map[i];
        if (entry.type != MEMORY_MAP_ENTRY_USABLE &&\
            entry.type != MEMORY_MAP_ENTRY_USABLE_ACPI)
            continue;

        uintptr_t p_addr = entry.addr;
        uint64_t length = entry.length;

        if (!CHECK_PAGE_4K_ALIGN(p_addr))
            p_addr = PAGE_4K_ALIGN(p_addr);
        if (!CHECK_PAGE_4K_ALIGN(length))
            length = PAGE_4K_ALIGN_DOWN(length);

        uint64_t max_local_index = PAGE_COUNT_4K_CONTAINING_LEN(length)-1;
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

    for (uint32_t i=0; i<pmm_state.memory_map_count; i++) {
        struct memory_map_entry entry = pmm_state.memory_map[i];
        if (entry.addr >= pmm_state.p_top) break;
        if (entry.type != MEMORY_MAP_ENTRY_USABLE &&\
            entry.type != MEMORY_MAP_ENTRY_USABLE_ACPI)
            continue;

        uintptr_t p_addr = entry.addr;
        uint64_t length = entry.length;

        /* Make sure page alignment is there */
        if (!CHECK_PAGE_4K_ALIGN(p_addr)) {
            uint64_t end = p_addr+length;
            p_addr = PAGE_4K_ALIGN(p_addr);
            length = end-p_addr;
        }
        /* There may be some wastage near the end of the mapping
         * if length is not page aligned, but will always be lesser than 4KB */
        if (!CHECK_PAGE_4K_ALIGN(length))
            length = PAGE_4K_ALIGN_DOWN(length);

        if (p_addr + length - 1 < pmm_state.p_bottom)
            continue;

        if (p_addr < pmm_state.p_bottom) {
            length = p_addr+length-pmm_state.p_bottom;
            p_addr = pmm_state.p_bottom;
        }

        if (p_addr + length - 1 >= pmm_state.p_top) 
            length -= p_addr+length - pmm_state.p_top;

        /* We have accounted for bottom/top trimming, and this is the right
         * time to set virtual memory mirror for usable physical zone */
        int s = paging_set_map_best_fit(pmm_state.mirror_base+p_addr, p_addr, length, PAGE_1G, true);
        if (s) {
            vga_print_uint(s, 10, -1);
            vga_print("\n");
            return 11;
        }

        /* Handle the case of allocating page list inside
         * this zone, as early as possible, if it is large enough
         * to contain it. We skip over it for zone init */
        if (!list_allocated && length >= list_size) {
            int s = pmm_allocate_lists(p_addr, zone_count, list_size);

            /* Error occured during allocation */
            if (s) 
                return s;
            p_addr = pmm_p_ptr(pmm_state.page_list_top);
            length -= list_size;
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
                if (!CHECK_ORDER_ALIGN(p_addr, i)) {
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
 
            pmm_top_level_block(p_addr, order);

            uint64_t block_length = PAGE_4K<<order;

            length -= block_length;
            p_addr += block_length; 
        }
    }

    /* Suitable region couldnt be found to allocate lists */
    if (!list_allocated)
        return 2;

    return 0;
}

/* Get memory map in format specified in bootinfo.h,
 * get base physical address below which we dont touch memory */
int pmm_initialize(struct memory_map_entry* entries, uint32_t entry_count, uintptr_t bottom, uintptr_t top, uint64_t mirror_base) {
    pmm_state.memory_map = entries;
    pmm_state.memory_map_count = entry_count;
    pmm_state.p_bottom = bottom;
    pmm_state.p_top = top;
    pmm_state.mirror_base = mirror_base;

    if (!CHECK_PAGE_4K_ALIGN(pmm_state.p_bottom))
        pmm_state.p_bottom = PAGE_4K_ALIGN(pmm_state.p_bottom);
    if (!CHECK_PAGE_4K_ALIGN(pmm_state.p_top))
        pmm_state.p_top = PAGE_4K_ALIGN_DOWN(pmm_state.p_top);

    for (int i=0; i<18; i++) {
        pmm_state.free_lists[i] = NULL;
    }

    int s = pmm_initialize_zones();
    if (s)
        return s;
   
    vga_print("[");vga_print_color("Physical Memory Manager",VGA_COLOR_LIGHT_GREEN);vga_print("] ");
    vga_print("Initialized\n");
    vga_print("    Bottom: ");
    vga_print_uint_color(pmm_state.p_bottom, 16, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print(" | Top: ");
    vga_print_uint_color(pmm_state.p_top, 16, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");

    return 0;
}

/* Allocate a 4KB page from physical memory, return NULL if not possible */
void* pmm_allocate_page() {
    if (pmm_state.memory_map == NULL) return NULL;

    return NULL;
}

uintptr_t pmm_p_ptr(void* v_ptr) {
    if (v_ptr == NULL) return 0;
    return (~pmm_state.mirror_base)&(uint64_t)v_ptr;
}

void* pmm_v_ptr(uintptr_t p_ptr) {
    if (p_ptr == 0) return NULL;
    return (void*)(pmm_state.mirror_base|(uint64_t)p_ptr);
}
