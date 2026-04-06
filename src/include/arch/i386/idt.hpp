#pragma once

#include "utils/types.hpp"

/**
 * @brief Interrupt Descriptor Table Entry
 */
struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

/**
 * @brief IDT Pointer structure for 'lidt' instruction
 */
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/**
 * @brief Interrupt Descriptor Table Manager
 * 
 * Manages the interrupt vector table, mapping hardware and software 
 * interrupts to specific handler functions.
 */
class IDT {
public:
    /**
     * @brief Initialize IDT and load it into the CPU
     */
    static void initialize();

    /**
     * @brief Register an interrupt handler gate
     */
    static void set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

private:
    static idt_entry entries[256];
    static idt_ptr ptr;
};
