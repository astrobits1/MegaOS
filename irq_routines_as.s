.section text
.code32

.global irq0_division_error_fault
.type irq0_division_error_fault, @function
irq0_division_error_fault:
    
    iret


