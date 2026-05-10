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

    /* Initialize CPU level paging functionality in order to write to physical
     * memory as we are currently in virtually mapped space */
    paging_initialize_allocator(bump_allocate_page, bump_free_page, bump_get_physical);

    /* Get the already loaded PML4 and load it in our paging manager */
    volatile void* pml4 = get_pml4();
    paging_set_pml4(pml4);

    /* Initialize linear allocator for allocation during PMM bootstrap 
     * Only 32KB is permitted as this should be enough for basic page table
     * allocation and writing */
    const uint64_t bump_alloc_size = 0x8000;
    uint64_t bottom = info->kernel_physical.end;
    uint64_t top = info->kernel_physical.end+bump_alloc_size;
    uint64_t v_base = (uint64_t)_KERNEL_END;

    if (!CHECK_PAGE_4K_ALIGN(bottom))
        bottom = PAGE_4K_ALIGN(bottom);
    if (!CHECK_PAGE_4K_ALIGN(top))
        top = PAGE_4K_ALIGN_DOWN(top);
    if (!CHECK_PAGE_4K_ALIGN(v_base))
        v_base = PAGE_4K_ALIGN(v_base);

    /* TODO Kernel must stage PML4 sub-structure in bump allocator
     * to take ownership of them */

    /* This maps the allocator region physical bottom to a virtual address of our choice 
     * TODO
     * This will cause a crash as it unmaps core kernel 2MB page, boot32 must only map till 
     * kernel end */
    bump_initialize(info->kernel_physical.end, info->kernel_physical.end+bump_alloc_size, (uint64_t)_KERNEL_END);

    /* Allocation of new pages should work fine and any new changes are 
     * appended to the already loaded PML4 */

    /* Starting from the end of the kernel, map the entire possible memory map
     * Kernel and low memory is intentionally left out from the PMM as a safety measure */
    int s = pmm_initialize((void*)info->map_entries, info->map_entry_count, \
            info->kernel_physical.end+1+bump_alloc_size, UINT64_MAX);
    if (s > 0) {
        vga_print_color("Fatal PMM error during initialization\n", VGA_COLOR_RED);
        goto panic;
    }

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
