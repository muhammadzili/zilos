#pragma once

#include "utils/types.hpp"

/**
 * @brief CPU Register State during an Interrupt
 * 
 * Matches the layout pushed by the common ISR stub in interrupts.s
 */
struct registers {
    uint32_t ds;                                     // Data segment selector
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by 'pusha'
    uint32_t int_no, err_code;                       // Interrupt number and error code (if any)
    uint32_t eip, cs, eflags, useresp, ss;           // Pushed by the processor automatically
};

/**
 * @brief Interrupt Service Routine pointer type
 */
typedef void (*isr_t)(registers*);

/**
 * @brief Register a high-level handler for a specific interrupt vector
 */
void register_interrupt_handler(uint8_t n, isr_t handler);

// --- Low-Level Handlers (called from assembly) ---
extern "C" void isr_handler(registers* r);
extern "C" void irq_handler(registers* r);
