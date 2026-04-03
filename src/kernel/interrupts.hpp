#pragma once
#include "utils/types.hpp"

struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

typedef void (*isr_t)(registers*);

extern "C" void isr_handler(registers* r);
extern "C" void irq_handler(registers* r);

void register_interrupt_handler(uint8_t n, isr_t handler);
