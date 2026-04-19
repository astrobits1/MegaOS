#include <boot32/boot.h>
#include <boot32/paging.h>
#include <boot32/vga.h>

extern volatile const void* _bootstart;
extern volatile const void* _bootend;

#define M 52
/* 4KB */
#define MAP_SIZE 0x1000

/* Linear allocator for page structures */

volatile void* allocation_pointer = NULL;

volatile void* allocate_map() {
    /* Allocate 4KB page map layer (table/directory/pdpt/pml4) and 
     * return its address */
    if (allocation_pointer == NULL) {
        /* Initialise to closest 4KB page aligned physical address that doesnt overlap kernel
         * After the kernel */
        allocation_pointer = (void*)((uint32_t)(_bootend)&(~(MAP_SIZE-1)))+MAP_SIZE;
    }

    volatile void* object = allocation_pointer;
    allocation_pointer = (void*)(((uint32_t)allocation_pointer)+MAP_SIZE);

    return object;
}

void initialise_map(volatile uint8_t* map) {
    /* Initialise map to unmapped by default, (P bit 0) */

    for (uint32_t i=0; i<MAP_SIZE; i++) {
        map[i] = 0;
    }
}

#define PDE_2M_PAGE_MASK ~0x1FFFFF
#define PDPT_1G_PAGE_MASK ~0x3FFFFFFF
#define NO_PAGE_MASK 0
#define SET_PAGESIZE 1
#define NOSET_PAGESIZE 0

/* Generalised function used to write address to pt, pd, pdpt, pml4
 *
 * pd and pdpt support pagesize bit which lets you define a contigious chunk of 2MB or 1GB 
 * in physical memory as a page. You write the physical memory address using SET_PAGESIZE and
 * the appropriate mask for them, otherwise pass NOSET_PAGESIZE, and NO_PAGE_MASK
 *
 * Flag configuration is minimal and default as the main aim of setting up paging here
 * is to identity map kernel zone and enter long mode */

void write_map_entry(volatile uint8_t* map, uint16_t index, uint32_t a_hi, uint32_t a_lo, uint8_t pagesize, uint32_t pagesize_mask) {
    a_hi &= (1<<(M-32))-1;

    uint32_t flag_data = 0x003;
    if (pagesize) {
        /* Interpret physical address as single 2MB/1GB page (only for PD/PDTE) */
        flag_data |= 1 << 7;
        a_lo &= pagesize_mask;
    } else a_lo &= ~0xFFF;

    a_lo |= flag_data;

    volatile uint32_t* ptr = (volatile uint32_t*)&map[index*8];
    ptr[0] = a_lo;
    ptr[1] = a_hi;
}


void paging_initialise() {
    vga_print_color("Initializing 64 bit Paging\n", VGA_COLOR_LIGHT_GREEN);

    /* Allocate page maps in physical memory, in a region that is outside .data
     * right at the end of the kernel 
     * They should be put at a page aligned physical address
     * Each map is 4KB in size 
     *
     * All the page structures should be within identity mapped region
     * or modifying them will cause a page fault */

    volatile void* pml4 = allocate_map();        /* Page map layer 4 */
    volatile void* pdpt = allocate_map();        /* Page directory pointer table */
    volatile void* pd   = allocate_map();        /* Page directory */
    volatile void* pt   = allocate_map();        /* Page table (contains pointer to physical address) */

    initialise_map(pml4);
    initialise_map(pdpt);
    initialise_map(pd);
    initialise_map(pt);

    /* Identity map 0-4MB */
    write_map_entry(pd, 0, 0, 0, SET_PAGESIZE, PDE_2M_PAGE_MASK);
    write_map_entry(pd, 1, 0, 0x00200000, SET_PAGESIZE, PDE_2M_PAGE_MASK);

    /* Load page directory to ptpt and ptpt to pml4 */
    write_map_entry(pdpt, 0, 0, (uint32_t)pd, NOSET_PAGESIZE, NO_PAGE_MASK);
    write_map_entry(pml4, 0, 0, (uint32_t)pdpt, NOSET_PAGESIZE, NO_PAGE_MASK);

    vga_print("PML4*: "); vga_print_u32((uint32_t)pml4, 16, 8);
    vga_print(", PDPT*: "); vga_print_u32((uint32_t)pdpt, 16, 8);
    vga_print(", PD*: "); vga_print_u32((uint32_t)pd, 16, 8);
    vga_print("\n");

    /* Load pml4 to CR3 */
    load_pml4(pml4);
    vga_print("Loaded PML4 to CR3\n");
    /* Enable 64 bit paging, and compatibility mode 
     * kernel and surrounding space should be identity mapped */
    enable_paging64();
    vga_print_color("Enabled 64 bit Paging and x86_64 compatibility mode\n", VGA_COLOR_LIGHT_GREEN);
}

