#include <kernel64/memory/paging.h>
#include <kernel64/memory/bump.h>
#include <common/drivers/vga/vga.h>
#include <stddef.h>

/* Free pages are used as linked list objects in the free list
 * to create a list of free pages that can be reallocated on demand */
struct bump_free_page {
    struct bump_free_page* next;
};

struct bump_pvt {
    /* Physical address */
    uintptr_t bottom;
    uintptr_t top;

    /* Virtual address */
    uint64_t allocation_pointer;
    uint64_t v_base;
    struct bump_free_page* free_list;
};


struct bump_pvt bump_state;

int bump_initialize(uintptr_t bottom, uintptr_t top, uint64_t v_base) {
    bump_state.bottom = bottom;
    bump_state.top = top;
    bump_state.v_base = v_base;
    bump_state.free_list = NULL;

    if (!CHECK_PAGE_4K_ALIGN(bump_state.bottom))
        bump_state.bottom = PAGE_4K_ALIGN(bump_state.bottom);
    if (!CHECK_PAGE_4K_ALIGN(bump_state.top))
        bump_state.top = PAGE_4K_ALIGN_DOWN(bump_state.top);
    if (!CHECK_PAGE_4K_ALIGN(v_base))
        bump_state.v_base = PAGE_4K_ALIGN(bump_state.v_base);

    bump_state.allocation_pointer = bump_state.v_base;

    vga_print("[");vga_print_color("Bump Allocator",VGA_COLOR_LIGHT_GREEN);vga_print("] ");
    vga_print("Initialized\n");
    vga_print("    Bottom: ");
    vga_print_uint_color(bump_state.bottom, 16, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print(" | Top: ");
    vga_print_uint_color(bump_state.top, 16, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");

    return 0;
}

void bump_uninitialize() {
    uint64_t page_count = (bump_state.top-bump_state.bottom)>>12;
    paging_unmap(bump_state.v_base, PAGE_4K, page_count);

    bump_state.bottom = 0;
    bump_state.top = 0;
    bump_state.free_list = NULL;
    bump_state.allocation_pointer = 0;
    bump_state.v_base = 0;
}

/* Linear allocation,
 * This primarily serves as an allocator for early boot
 * before proper memory management takes over */
void* bump_allocate_page() {
    if (bump_state.allocation_pointer == 0)
        return NULL;

    /* Reuse page from freelist when possible */
    if (bump_state.free_list != NULL) {
        struct bump_free_page* new_head = bump_state.free_list->next;
        void* allocated = bump_state.free_list;
        bump_state.free_list = new_head;

        return allocated;
    }

    if (bump_state.allocation_pointer + PAGE_4K >= bump_state.top)
        return NULL;

    void* object = (void*)(uintptr_t)bump_state.allocation_pointer;
    bump_state.allocation_pointer += PAGE_4K;

    return object;
}

/* Freeing adds the page to the free list, ready for reuse
 * This creates fragmentation quickly but is alright for early bootstrapping */
void bump_free_page(void* page) {
    struct bump_free_page* head = bump_state.free_list;
    struct bump_free_page* new = (struct bump_free_page*)page;

    new->next = head;
    bump_state.free_list = new;
}

/* Convert virtual address of page to physical address */
void* bump_get_physical(void* page) {
    return (void*)(uintptr_t)(bump_state.bottom+((uintptr_t)page-bump_state.v_base));
}
