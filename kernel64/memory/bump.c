#include <boot32/boot.h>
#include <kernel64/memory/paging.h>
#include <kernel64/memory/bump.h>
#include <stddef.h>

uint64_t BOTTOM = 0;
uint64_t TOP = 0;
uint64_t allocation_pointer = 0;

void bump_initialize(uint64_t bottom, uint64_t top) {
    BOTTOM = bottom;
    TOP = top;
    allocation_pointer = bottom;

    if (!CHECK_PAGE_4K_ALIGN(BOTTOM))
        BOTTOM = PAGE_4K_ALIGN(BOTTOM);
    if (!CHECK_PAGE_4K_ALIGN(TOP))
        TOP = PAGE_4K_ALIGN_DOWN(TOP);
}

/* Linear allocation,
 * This primarily serves as an allocator for early boot
 * before proper memory management takes over */
void* bump_allocate_page() {
    if (allocation_pointer == 0)
        return NULL;

    if (allocation_pointer + PAGE_4K >= TOP)
        return NULL;

    void* object = (void*)(uintptr_t)allocation_pointer;
    allocation_pointer += PAGE_4K;

    return object;
}

/* TODO */
void bump_free_page(void* page) {

}
