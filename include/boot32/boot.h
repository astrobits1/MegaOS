#ifndef BOOT32_BOOT_H
#define BOOT32_BOOT_H

#include <stdint.h>
#include <boot32/mbi2.h>

/* Byte immediately at the beginning and after the end of the kernel in physical memory,
 * marked in linker script */

extern uint8_t _BOOT_BEGIN;
extern uint8_t _BOOT_END;

__attribute__((noreturn))
void boot_panic();

/* longmode.s */
void jump_kernel(uint32_t entry_lo, uint32_t entry_hi, struct bootinfo* info);
void enable_sse();

#endif
