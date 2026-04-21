#include <boot32/boot.h>
#include <boot32/paging.h>
#include <common/drivers/vga/vga.h>
#include <boot32/allocator.h>


/* Map references for initial 0-1GB */
volatile void* PML4 = NULL;


void initialise_map(volatile uint8_t* map) {
    /* Initialise map to unmapped by default, (P bit 0) */

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
 * Flag configuration is minimal and default as the main aim of setting up paging here
 * is to identity map kernel zone and enter long mode */

void write_map_entry(volatile uint8_t* map, uint16_t index, uint32_t addr, uint8_t pagesize, uint32_t pagesize_mask) {
    uint32_t a_lo = addr & 0xFFFFFFFF;

    uint32_t flag_data = 0x003;
    if (pagesize) {
        /* Interpret physical address as single 2MB/1GB page (only for PD/PDTE) */
        flag_data |= 1 << 7;
        a_lo &= pagesize_mask;
    } else a_lo &= ~0xFFF;

    a_lo |= flag_data;

    uint32_t* ptr = (uint32_t*)&map[index*8];
    ptr[0] = a_lo;
    ptr[1] = 0;
}

/* Read address in entry at index in map, and get pagesize info */

uint32_t read_map_entry(volatile uint8_t* map, uint16_t index, uint8_t* pagesize) { 
    uint32_t* ptr = (uint32_t*)&map[index*8];
    uint32_t a_lo = ptr[0];

    *pagesize = a_lo >> 7 & 1;
    /* Remove flag bits */
    a_lo &= ~0xFFF;
    return a_lo;
}

bool check_map_entry_present(volatile uint8_t* map, uint16_t index) {
    return (bool)(map[index*8] & 1);
}

/* Linear map virtual address to given physical address in 2MB page blocks,
 * count represents no. of 2MB blocks starting from physical and mapped to virtual 1:1 */

void map_memory_2M(uint64_t v_addr, uint32_t p_addr, uint8_t count) {
    /* Virtual and physical must be page aligned */
    if ((v_addr & PDE_2M_PAGE_MASK) != v_addr || (p_addr & PDE_2M_PAGE_MASK) != p_addr) {
        vga_print_color("Virtual and Physical address must be 2MB page aligned\n", VGA_COLOR_RED);
        boot_panic();
        return;
    }

    /* Decode virtual address into PML4, PDPT, PD indices
     * If maps at those indices dont exist then create them */
    uint16_t pde = v_addr >> 21 & 0x1FF;
    uint16_t pdpte = v_addr >> 30 & 0x1FF;
    uint16_t pml4e = v_addr >> 39 & 0x1FF;

    volatile void* pd = NULL;
    volatile void* pdpt = NULL;

    
    for (uint8_t i=0; i<count; i++) {
        /* PML4E decoding */
        if (!check_map_entry_present(PML4, pml4e)) {
            volatile void* map = allocator_alloc_page();
            initialise_map(map);

            write_map_entry(PML4, pml4e, (uint32_t)map, NOSET_PAGESIZE, NO_PAGE_MASK);
            pdpt = map;
        } else {
            uint8_t PS;
            pdpt = (volatile void*)read_map_entry(PML4, pml4e, &PS);
        }

        /* PDPTE decoding */
        if (!check_map_entry_present(pdpt, pdpte)) {
            volatile void* map = allocator_alloc_page();
            initialise_map(map);

            write_map_entry(pdpt, pdpte, (uint32_t)map, NOSET_PAGESIZE, NO_PAGE_MASK);
            pd = map;
        } else {
            uint8_t PS;
            pd = (volatile void*)read_map_entry(pdpt, pdpte, &PS);
        }

        /* PD write */
        write_map_entry(pd, pde, p_addr, SET_PAGESIZE, PDE_2M_PAGE_MASK); 

        if (((uint64_t)p_addr + PAGE_2M) > UINT32_MAX) {
            vga_print_color("Physical address overflow while mapping\n", VGA_COLOR_RED);
            boot_panic();
            return;
        }

        p_addr += PAGE_2M;
        pde += 1;

        /* Handle overflow into next maps */
        if (pde == MAP_ENTRY_COUNT) {
            pde = 0;
            pdpte += 1;
            if (pdpte == MAP_ENTRY_COUNT) {
                pdpte = 0;
                pml4e += 1;
                if (pml4e == MAP_ENTRY_COUNT) {
                    vga_print_color("Virtual address overflow while mapping\n", VGA_COLOR_RED);
                    boot_panic();
                    return;
                }
            }
        }
    }
}

void paging_initialise() {
    //vga_print_color("Initializing 64 bit Paging\n", VGA_COLOR_LIGHT_GREEN);

    /* Allocate page maps in physical memory, in a region that is outside .data
     * right at the end of the kernel 
     * They should be put at a page aligned physical address
     * Each map is 4KB in size 
     *
     * All the page structures should be within identity mapped region
     * or modifying them will cause a page fault */

    volatile void* pml4 = allocator_alloc_page();        /* Page map layer 4 */

    initialise_map(pml4);
    PML4 = pml4;

    /* Identity map first 4 MB that consists of boot kernel + allocation limit */
    map_memory_2M(0, 0, 2); 

    /* Load pml4 to CR3 */
    load_pml4(pml4);
    PML4 = pml4;

    /* Enable 64 bit paging, and compatibility mode 
     * kernel and surrounding space should be identity mapped */
    enable_paging64();
    vga_print_color("Enabled 64 bit Paging and x86_64 compatibility mode\n", VGA_COLOR_LIGHT_GREEN);
}

