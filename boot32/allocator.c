#include <boot32/allocator.h>
#include <boot32/boot.h>
#include <common/drivers/vga/vga.h>
#include <stddef.h>

extern const void* _bootstart;
extern const void* _bootend;

void* heap_bottom = NULL;
void* allocation_pointer = NULL;
void* heap_top = NULL;

void allocator_initialise(void* bottom, void* top) {
    heap_bottom = bottom;
    heap_top = top;
    allocation_pointer = bottom;
}

void* allocator_alloc_page() {
    if ((uint32_t)(allocation_pointer) + PAGE_4K > (uint32_t)heap_top) {
        vga_setfgcolor(VGA_COLOR_RED);
        vga_print("Allocator overflowed: \nHeap bottom: "); vga_print_uint((uint32_t)heap_bottom, 16, 8);
        vga_print("Heap top: "); vga_print_uint((uint32_t)heap_top, 16, 8);
        vga_print("\n");
        vga_print("Allocation pointer: "); vga_print_uint((uint32_t)allocation_pointer, 16, 8);
        vga_print("\n");
        boot_panic();

        return NULL;
    }

    void* object = allocation_pointer;
    allocation_pointer = (void*)(((uint32_t)allocation_pointer)+PAGE_4K);

    return (void*)object;
}
