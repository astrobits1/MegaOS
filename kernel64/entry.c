#include <kernel64/state.h>
#include <kernel64/cpu/gdt.h>
#include <kernel64/cpu/idt.h>
#include <kernel64/memory/paging.h>
#include <kernel64/memory/bump.h>
#include <kernel64/memory/pmm.h>
#include <common/bootinfo.h>
#include <common/drivers/vga/vga.h>

/* paging_as.s */
extern void* get_pml4();

void kernel_setup(struct bootinfo* info) {
    /* Put the kernel in a stable long mode state */

    /* Set up kernel and user mode code and data segments + TSS (TODO)*/
    gdt_initialise();

    /* Initialise 64 bit IDT and link exception handlers */
    idt_initialise();  

    /* Initialize linear allocator for allocation during PMM bootstrap */
    uint64_t bottom = info->kernel_physical.end;
    /* Ideally this should be a value in bootinfo that tells us
     * how much extra space was mapped, but 1MB is an agreement for now TODO */
    uint64_t top = info->kernel_physical.end+0x100000;
    uint64_t v_base = (uint64_t)_KERNEL_END;

    if (!CHECK_PAGE_4K_ALIGN(bottom))
        bottom = PAGE_4K_ALIGN(bottom);
    if (!CHECK_PAGE_4K_ALIGN(top))
        top = PAGE_4K_ALIGN_DOWN(top);
    if (!CHECK_PAGE_4K_ALIGN(v_base))
        v_base = PAGE_4K_ALIGN(v_base);

    /* TODO Kernel must stage PML4 sub-structure in bump allocator
     * to take ownership of them 
     *
     * IMP
     * We cannot free any map allocated by boot32, as it is out of allocation
     * area and address translations will fail if it is freed and reallocated.
     * So we cant overwrite any mapping set up by boot32, in the current PML4 */

    /* This expects a map of the allocator region physical bottom to a virtual address 
     * of our choice which is the kernel virtual end in this case.
     *
     * The map should already be present because bootloader has mapped some additional
     * space after kernel in virtual and told us how much. Which is what top is adjusted to */
    bump_initialize(bottom, top, v_base);
    /* bump allocator can be supplied to paging API now */

    /* Initialize allocator for paging  */
    paging_initialize_allocator(bump_allocate_page, bump_free_page, bump_p_ptr, bump_v_ptr);
    /* Paging API can be used now */

    /* Get the already loaded PML4 and load it in our paging manager */
    volatile void* pml4 = get_pml4();
    paging_set_pml4(pml4);
    /* Allocation of new pages should work fine and any new changes are 
     * appended to the already loaded PML4 */

    /* Starting from the end of the kernel, map the entire possible memory map
     * Kernel and low memory is intentionally left out from the PMM as a safety measure */
    int s = pmm_initialize((void*)info->map_entries, info->map_entry_count, \
            top+1, STATE_PMM_MAX_P_ADDR, STATE_V_PMM_BASE);
    if (s > 0) {
        vga_print_color("Fatal PMM error during initialization\n", VGA_COLOR_RED);
        goto panic;
    }
    
    /* PMM is initialized */
panic:
    kernel_panic();
}

__attribute__((noreturn))
void kernel_main(struct bootinfo* info) {
    vga_initialize();
    vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_clear();

    vga_print_color("Entered long mode\n", VGA_COLOR_LIGHT_MAGENTA);
    vga_print("Welcome to Tesseract OS\n\n");

    bootinfo_print_memory_map(info);

    kernel_setup(info);

    lock();
}
