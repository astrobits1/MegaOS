#include <kernel64/state.h>

__attribute__((noreturn))
void lock() {
l:
    asm volatile("cli; hlt");
    goto l;
}

__attribute__((noreturn))
void kernel_panic() {
    lock();
}
