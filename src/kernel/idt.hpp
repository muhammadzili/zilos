#pragma once
#include "utils/types.hpp"

struct idt_entry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

class IDT {
public:
    static void initialize();
    static void set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
private:
    static idt_entry entries[256];
    static idt_ptr ptr;
};
