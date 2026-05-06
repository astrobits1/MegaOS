#include <common/memory/pmm.h>
#include <common/memory/vmm.h>
#include <common/drivers/vga/vga.h>

/* Map references for initial 0-1GB */
volatile void* vmm_PML4 = NULL;

/* Set top level pml4 */
void vmm_set_pml4(volatile void* pml4) {
    vmm_PML4 = pml4;
}

/* Initialise map to unmapped by default, (P bit 0) */
void vmm_initialise_map(volatile uint8_t* map) {
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
void vmm_write_map_entry(volatile uint8_t* map, uint16_t index, uint64_t addr, uint8_t pagesize, uint64_t pagesize_mask) {
    uint64_t a = addr&(((uint64_t)1<<M)-1);

    uint32_t flag_data = 0x003;
    if (pagesize) {
        /* Interpret physical address as single 2MB/1GB page (only for PD/PDTE) */
        flag_data |= 1 << 7;
        a &= pagesize_mask;
    } else a &= ~0xFFF;

    a |= flag_data;

    uint64_t* ptr = (uint64_t*)&map[index*8];
    *ptr = a;
}

/* Read address in entry at index in map, and get pagesize info */
uint64_t vmm_read_map_entry(volatile uint8_t* map, uint16_t index, uint8_t* pagesize) { 
    uint64_t* ptr = (uint64_t*)&map[index*8];
    uint64_t a = ptr[0];

    *pagesize = a >> 7 & 1;
    /* Remove flag bits */
    a &= ~0xFFF;

    /* Address is M bits long */
    a &= ((uint64_t)1<<M)-1;
    return a;
}

bool vmm_check_map_entry_present(volatile uint8_t* map, uint16_t index) {
    return (bool)(map[index*8] & 1);
}

/* Linear map virtual address to given physical address in 2MB page blocks,
 * count represents no. of 2MB blocks starting from physical and mapped to virtual 1:1 */
int vmm_map_memory_2M(uintptr_t v_addr, uintptr_t p_addr, uint8_t count) {
    /* Virtual and physical must be page aligned */
    if ((v_addr & PDE_2M_PAGE_MASK) != v_addr || (p_addr & PDE_2M_PAGE_MASK) != p_addr) {
        vga_print_color("Virtual and Physical address must be 2MB page aligned\n", VGA_COLOR_RED);
        return 1;
    }

    /* Decode virtual address into PML4, PDPT, PD indices
     * If maps at those indices dont exist then create them */
    uint16_t pde = v_addr >> 21 & 0x1FF;
    uint16_t pdpte = v_addr >> 30 & 0x1FF;
    uint16_t pml4e = (uint64_t)v_addr >> 39 & 0x1FF;

    volatile void* pd = NULL;
    volatile void* pdpt = NULL;
    
    for (uint8_t i=0; i<count; i++) {
        /* PML4E decoding */
        if (!vmm_check_map_entry_present(vmm_PML4, pml4e)) {
            volatile void* map = pmm_allocate_page();
            vmm_initialise_map(map);

            vmm_write_map_entry(vmm_PML4, pml4e, (uintptr_t)map, NOSET_PAGESIZE, NO_PAGE_MASK);
            pdpt = map;
        } else {
            uint8_t PS;
            pdpt = (volatile void*)(uintptr_t)vmm_read_map_entry(vmm_PML4, pml4e, &PS);
        }

        /* PDPTE decoding */
        if (!vmm_check_map_entry_present(pdpt, pdpte)) {
            volatile void* map = pmm_allocate_page();
            vmm_initialise_map(map);

            vmm_write_map_entry(pdpt, pdpte, (uintptr_t)map, NOSET_PAGESIZE, NO_PAGE_MASK);
            pd = map;
        } else {
            uint8_t PS;
            pd = (volatile void*)(uintptr_t)vmm_read_map_entry(pdpt, pdpte, &PS);
        }

        /* PD write */
        vmm_write_map_entry(pd, pde, p_addr, SET_PAGESIZE, PDE_2M_PAGE_MASK); 

        if ((p_addr + PAGE_2M) < p_addr) {
            vga_print_color("Physical address overflow while mapping\n", VGA_COLOR_RED);
            return 2;
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
                    return 3;
                }
            }
        }
    }

    return 0;
}
