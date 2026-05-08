#include <kernel64/memory/paging.h>
#include <kernel64/memory/bump.h>
#include <stddef.h>

/* Free pages are used as linked list objects in the free list
 * to create a list of free pages that can be reallocated on demand */
struct bump_free_page {
    struct bump_free_page* next;
};

struct bump_state {
    uint64_t bottom;
    uint64_t top;
    uint64_t allocation_pointer;

    struct bump_free_page* free_list;
};


struct bump_state state;

void bump_initialize(uint64_t bottom, uint64_t top) {
    state.bottom = bottom;
    state.top = top;
    state.allocation_pointer = bottom;
    state.free_list = NULL;

    if (!CHECK_PAGE_4K_ALIGN(state.bottom))
        state.bottom = PAGE_4K_ALIGN(state.bottom);
    if (!CHECK_PAGE_4K_ALIGN(state.top))
        state.top = PAGE_4K_ALIGN_DOWN(state.top);
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
