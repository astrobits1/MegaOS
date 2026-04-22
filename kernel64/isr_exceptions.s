.macro push_regs_setup_frame
    push %rbp
    mov %rsp, %rbp
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15
.endm

.macro pop_regs
    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax
    pop %rbp
.endm


/* Causes panic to occur on exception, pushes exception no., interrupt frame pointer and error code
if applicable */

.macro isr_panic_no_err_stub x
isr\x\()_exception:
    push_regs_setup_frame

    /* Exception index as 1st arg */ 
    mov $\x, %rdi
    /* Interrupt frame pointer as 2st arg */
    mov %rbp, %rsi
    add $8, %rsi
    call exception_handler_panic_no_err

    pop_regs
    iretq
.endm

.macro isr_panic_err_stub x
isr\x\()_exception:
    push_regs_setup_frame

    /* Exception index as 1st arg */
    mov $\x, %rdi

    /* Error code as 2nd arg */
    mov 16(%rbp), %rsi

    /* Interrupt frame pointer as 3rd arg */
    mov %rbp, %rdx
    add $8, %rdx

    call exception_handler_panic_err

    pop_regs
    iretq
.endm

/* Add specific stubs for particular exceptions as I add them */



/* Default unimplemented exceptions to panic */
.section .text
.code64

isr_panic_no_err_stub 0
isr_panic_no_err_stub 1
isr_panic_no_err_stub 2
isr_panic_no_err_stub 3
isr_panic_no_err_stub 4
isr_panic_no_err_stub 5
isr_panic_no_err_stub 6
isr_panic_no_err_stub 7
isr_panic_err_stub    8
isr_panic_no_err_stub 9
isr_panic_err_stub    10
isr_panic_err_stub    11
isr_panic_err_stub    12
isr_panic_err_stub    13
isr_panic_err_stub    14
isr_panic_no_err_stub 15
isr_panic_no_err_stub 16
isr_panic_err_stub    17
isr_panic_no_err_stub 18
isr_panic_no_err_stub 19
isr_panic_no_err_stub 20
isr_panic_err_stub    21
isr_panic_no_err_stub 22
isr_panic_no_err_stub 23
isr_panic_no_err_stub 24
isr_panic_no_err_stub 25
isr_panic_no_err_stub 26
isr_panic_no_err_stub 27
isr_panic_no_err_stub 28
isr_panic_no_err_stub 29
isr_panic_no_err_stub 30
isr_panic_no_err_stub 31


.section .data

/* Create pointer array of addresses of all isr handlers from 0-31 */
.global isr_exception_table
isr_exception_table:
.irp i,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    .quad isr\i\()_exception
.endr
