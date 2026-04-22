#ifndef ISR_CONST_COMMON_H
#define ISR_CONST_COMMON_H

static const char* exception_string[23] = {
    "Division error",
    "", "", "", "",
    "Bound range exceeded",
    "Invalid opcode",
    "Device not available",
    "Double fault",
    "Co-Processor segment overrun",
    "Invalid task switch segment",
    "Segment not present",
    "Stack segmentation fault",
    "General protection fault",
    "Page fault",
    "Reserved CPU exception (illegal)",
    "FPU floating point fault",
    "Alignment error fault",
    "Machine check abort",
    "SIMD floating point exception",
    "Virtualization exception",
    "Control protection exception",

    /* 22-31 */
    "Reserved CPU exception"
};


#endif
