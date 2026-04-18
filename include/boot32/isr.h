#ifndef BOOT32_IRQ_ROUTINES_H
#define BOOT32_IRQ_ROUTINES_H

extern void (*isr_exception_table[32])(void);

void unimplemented_routine();

#endif
