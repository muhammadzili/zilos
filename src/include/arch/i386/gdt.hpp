#pragma once

#include "utils/types.hpp"

/**
 * @brief Global Descriptor Table Entry
 */
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

/**
 * @brief GDT Pointer structure for 'lgdt' instruction
 */
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

/**
 * @brief Global Descriptor Table Manager
 * 
 * Handles memory segmentation and access rights for the kernel and user space.
 */
class GDT {
public:
    /**
     * @brief Initialize GDT and load it into the CPU
     */
    static void initialize();

private:
    static void set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
    
    static gdt_entry entries[5];
    static gdt_ptr ptr;
};
