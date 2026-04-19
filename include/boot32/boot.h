#ifndef BOOT32_BOOT_H
#define BOOT32_BOOT_H

#include <stdint.h>

/* Byte immediately at the beginning and after the end of the kernel in physical memory,
 * marked in linker script */

extern uint8_t _BOOT_BEGIN;
extern uint8_t _BOOT_END;

__attribute__((noreturn))
void boot_panic();

#endif
