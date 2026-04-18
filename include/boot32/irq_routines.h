#ifndef BOOT32_IRQ_ROUTINES_H
#define BOOT32_IRQ_ROUTINES_H

void irq0_division_error_fault();
void irq5_bound_range_exceed_fault();
void irq6_invalid_opcode_fault();
void irq7_dev_not_available_fault();
void irq8_double_fault();
void irq9_coprocessor_segment_overrun_fault();
void irq10_invalid_tss_fault();
void irq11_segment_not_present_fault();
void irq12_stack_segment_fault();
void irq13_general_protection_fault();
void irq14_page_fault();
void irq16_floating_point_fault();
void irq17_alignment_check_fault();
void unimplemented_fault();

void unimplemented_routine();

#endif
