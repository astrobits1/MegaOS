#ifndef KERNEL64_STATE_H
#define KERNEL64_STATE_H

#include <stdint.h>

#define STATE_V_PMM_BASE 0xFFFFE00000000000
#define STATE_PMM_MAX_P_ADDR 0x00000FFFFFFFFFFF

extern char _KERNEL_BEGIN[];
extern char _KERNEL_END[];

__attribute__((noreturn))
void lock();

__attribute__((noreturn))
void kernel_panic();

#endif
