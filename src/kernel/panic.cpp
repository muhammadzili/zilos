#include "kernel/panic.hpp"
#include "drivers/video/vga.hpp"

namespace Panic {

static PanicHandler custom_handler = 0;

void set_handler(PanicHandler handler) {
    custom_handler = handler;
}

static const char* exception_names[] = {
    "Divide Error", "Debug", "NMI", "Breakpoint",
    "Overflow", "BOUND Range", "Invalid Opcode", "Device Not Available",
    "Double Fault", "Coprocessor Segment", "Invalid TSS", "Segment Not Present",
    "Stack Fault", "General Protection", "Page Fault", "Reserved",
    "x87 FPU", "Alignment Check", "Machine Check", "SIMD FP", "Virtualization"
};

void halt(const char* msg) {
    asm volatile ("cli");
    
    VGA::clear();
    VGA::set_color(12);
    VGA::println("=== KERNEL PANIC ===");
    VGA::println(msg);
    
    if (custom_handler) {
        custom_handler(msg, 0);
    }
    
    VGA::println("");
    VGA::println("System halted.");
    
    while (true) {
        asm volatile ("hlt");
    }
}

void debug(const char* msg) {
    VGA::set_color(14);
    VGA::print("[DEBUG] ");
    VGA::set_color(15);
    VGA::println(msg);
}

}