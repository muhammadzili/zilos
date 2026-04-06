#include "arch/i386/interrupts.hpp"
#include "utils/ports.hpp"
#include "drivers/video/vga.hpp"

static isr_t interrupt_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

extern "C" void isr_handler(registers* r) {
    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    } else {
        VGA::print("Unhandled Exception: ");
        char buf[16];
        VGA::itoa(r->int_no, buf, 10);
        VGA::println(buf);
        while (1);
    }
}

extern "C" void irq_handler(registers* r) {
    // Send EOI to PIC
    if (r->int_no >= 40) {
        outb(0xA0, 0x20);
    }
    outb(0x20, 0x20);

    if (interrupt_handlers[r->int_no] != 0) {
        isr_t handler = interrupt_handlers[r->int_no];
        handler(r);
    }
}
