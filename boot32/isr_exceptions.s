/* All exceptions cause panic, as this is the boot kernel
 State preservation is not needed */

.macro isr_no_err_stub x
isr\x\()_exception:
    pushl $\x
    call panic_exception_handler_no_err
    iret
    /* No return */
.endm

.macro isr_err_stub x
isr\x\()_exception:
    pushl $\x
    call panic_exception_handler_err
    iret
    /* No return */
.endm


.section .text
.code32

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_err_stub    21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_no_err_stub 30
isr_no_err_stub 31



.section .data

/* Create pointer array of addresses of all isr handlers from 0-31 */
.global isr_exception_table
isr_exception_table:
.irp i,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    .long isr\i\()_exception
.endr
