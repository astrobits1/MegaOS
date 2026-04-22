#ifndef KERNEL64_STATE_H
#define KERNEL64_STATE_H

__attribute__((noreturn))
void lock();

__attribute__((noreturn))
void kernel_panic();

#endif
