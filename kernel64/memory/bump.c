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

    /* Map 'bottom' to v_base based on allocation allowance,
     * for now we only use 4K pages but this can be optimized
     * with huge pages (TODO).
     * Though bump allocator is only really used for small allocation
     * buffers for now */
    uint64_t page_count = (state.top-state.bottom)>>12;
    if (paging_map(state.v_base, state.bottom, PAGE_4K, page_count))
        return 1;

    state.allocation_pointer = state.v_base;

    vga_print("Bump allocator bottom: ");
    vga_print_uint(state.bottom, 16, -1);
    vga_print(", top: ");
    vga_print_uint(state.top, 16, -1);
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

    void* object = (void*)state.allocation_pointer;
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
    return (void*)(state.bottom+((uint64_t)page-state.v_base));
}
