#include "arch/i386/idt.hpp"
#include "utils/ports.hpp"

// static storage
idt_entry IDT::entries[256];
idt_ptr IDT::ptr;

// handlers from interrupts.s
extern "C" void idt_load(uint32_t);
extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();

extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();

// set_gate: config specific idt entry idc
void IDT::set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    entries[num].base_low  = (base & 0xFFFF);
    entries[num].base_high = (base >> 16) & 0xFFFF;
    entries[num].sel       = sel;
    entries[num].always0   = 0;
    entries[num].flags     = flags;
}

// init idt: remap pic and setup gates fr
void IDT::initialize() {
    ptr.limit = (sizeof(idt_entry) * 256) - 1;
    ptr.base  = (uint32_t)&entries;

    // pic remap
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // master 32
    outb(0xA1, 0x28); // slave 40
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    // exceptions (0-31)
    set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    set_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

    // irqs (32-47)
    set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    set_gate(47, (uint32_t)irq15, 0x08, 0x8E);

    // load and go
    idt_load((uint32_t)&ptr);
}
