#ifndef KERNEL64_ISR_H
#define KERNEL64_ISR_H

#include <stdint.h>

/* For kernel exceptions */
struct interrupt_frame {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
};

/* For user mode exceptions, rsp and ss is pushed additionally */

struct interrupt_frame_user {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

extern void (*isr_exception_table[32])(void);
extern void (*isr_interrupt_table[224])(void);

#endif
