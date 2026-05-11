#include <kernel64/memory/paging.h>
#include <kernel64/memory/bump.h>
#include <common/drivers/vga/vga.h>
#include <stddef.h>

/* Free pages are used as linked list objects in the free list
 * to create a list of free pages that can be reallocated on demand */
struct bump_free_page {
    struct bump_free_page* next;
};

struct bump_state {
    /* Physical address */
    uintptr_t bottom;
    uintptr_t top;

    /* Virtual address */
    uint64_t allocation_pointer;
    uint64_t v_base;
    struct bump_free_page* free_list;
};


struct bump_state state;

int bump_initialize(uintptr_t bottom, uintptr_t top, uint64_t v_base) {
    state.bottom = bottom;
    state.top = top;
    state.v_base = v_base;
    state.free_list = NULL;

    if (!CHECK_PAGE_4K_ALIGN(state.bottom))
        state.bottom = PAGE_4K_ALIGN(state.bottom);
    if (!CHECK_PAGE_4K_ALIGN(state.top))
        state.top = PAGE_4K_ALIGN_DOWN(state.top);
    if (!CHECK_PAGE_4K_ALIGN(v_base))
        state.v_base = PAGE_4K_ALIGN(state.v_base);

    state.allocation_pointer = state.v_base;

    vga_print("[");vga_print_color("Bump Allocator",VGA_COLOR_LIGHT_GREEN);vga_print("] ");
    vga_print("Initialized\n");
    vga_print("    Bottom: ");
    vga_print_uint_color(state.bottom, 16, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print(" | Top: ");
    vga_print_uint_color(state.top, 16, -1, VGA_COLOR_LIGHT_MAGENTA);
    vga_print("\n");

    return 0;
}

void bump_uninitialize() {
    uint64_t page_count = (state.top-state.bottom)>>12;
    paging_unmap(state.v_base, PAGE_4K, page_count);

    state.bottom = 0;
    state.top = 0;
    state.free_list = NULL;
    state.allocation_pointer = 0;
    state.v_base = 0;
}

/* Linear allocation,
 * This primarily serves as an allocator for early boot
 * before proper memory management takes over */
void* bump_allocate_page() {
    if (state.allocation_pointer == 0)
        return NULL;

    /* Reuse page from freelist when possible */
    if (state.free_list != NULL) {
        struct bump_free_page* new_head = state.free_list->next;
        void* allocated = state.free_list;
        state.free_list = new_head;

        return allocated;
    }

    if (state.allocation_pointer + PAGE_4K >= state.top)
        return NULL;

    void* object = (void*)(uintptr_t)state.allocation_pointer;
    state.allocation_pointer += PAGE_4K;

    return object;
}

/* Freeing adds the page to the free list, ready for reuse
 * This creates fragmentation quickly but is alright for early bootstrapping */
void bump_free_page(void* page) {
    struct bump_free_page* head = state.free_list;
    struct bump_free_page* new = (struct bump_free_page*)page;

    new->next = head;
    state.free_list = new;
}

/* Convert virtual address of page to physical address */
void* bump_get_physical(void* page) {
    return (void*)(uintptr_t)(state.bottom+((uintptr_t)page-state.v_base));
}
