#include "arch/i386/gdt.hpp"

// static storage
gdt_entry GDT::entries[5];
gdt_ptr GDT::ptr;

// from gdt.s
extern "C" void gdt_flush(uint32_t);

// set_gate: setup specific gdt entry idc
void GDT::set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    entries[num].base_low    = (base & 0xFFFF);
    entries[num].base_middle = (base >> 16) & 0xFF;
    entries[num].base_high   = (base >> 24) & 0xFF;

    entries[num].limit_low   = (limit & 0xFFFF);
    entries[num].granularity = (limit >> 16) & 0x0F;

    entries[num].granularity |= gran & 0xF0;
    entries[num].access      = access;
}

// init gdt: setup segments and load it fr
void GDT::initialize() {
    ptr.limit = (sizeof(gdt_entry) * 5) - 1;
    ptr.base  = (uint32_t)&entries;

    // config segments
    set_gate(0, 0, 0, 0, 0);                // null
    set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // k-code
    set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // k-data
    set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // u-code
    set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // u-data

    // flush to cpu
    gdt_flush((uint32_t)&ptr);
}
