#include <kernel64/memory/paging.h>
#include <kernel64/memory/bump.h>
#include <stddef.h>

struct bump_state {
    uint64_t bottom;
    uint64_t top;
    uint64_t allocation_pointer;
};

struct bump_state state;

void bump_initialize(uint64_t bottom, uint64_t top) {
    state.bottom = bottom;
    state.top = top;
    state.allocation_pointer = bottom;

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

    if (state.allocation_pointer + PAGE_4K >= state.top)
        return NULL;

    void* object = (void*)(uintptr_t)state.allocation_pointer;
    state.allocation_pointer += PAGE_4K;

    return object;
}

/* TODO */
void bump_free_page(void* page) {

}
