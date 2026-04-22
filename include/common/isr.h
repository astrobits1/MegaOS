#ifndef COMMON_ISR_H
#define COMMON_ISR_H


extern void (*isr_exception_table[32])(void);
void unimplemented_routine();

#endif
