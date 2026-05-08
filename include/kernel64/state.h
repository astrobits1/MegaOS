#ifndef KERNEL64_STATE_H
#define KERNEL64_STATE_H

#include <stdint.h>

extern char _KERNEL_BEGIN[];
extern char _KERNEL_END[];

__attribute__((noreturn))
void lock();

__attribute__((noreturn))
void kernel_panic();

#endif
