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

.macro load_interrupt_args x
    /* Exception index as 1st arg */ 
    mov $\x, %rdi
    /* Interrupt frame pointer as 2st arg */
    mov %rbp, %rsi
    add $8, %rsi
.endm

.macro load_exception_args_err x
    /* Exception index as 1st arg */
    mov $\x, %rdi

    /* Error code as 2nd arg */
    mov 16(%rbp), %rsi

    /* Interrupt frame pointer as 3rd arg */
    mov %rbp, %rdx
    add $8, %rdx
.endm
