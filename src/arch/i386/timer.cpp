#include "arch/i386/timer.hpp"
#include "utils/ports.hpp"
#include "arch/i386/interrupts.hpp"

static uint32_t timer_ticks = 0;
static void (*user_handler)(void) = 0;

extern "C" void timer_callback(registers* r) {
    timer_ticks++;
    if (user_handler) {
        user_handler();
    }
}

void Timer::initialize(uint32_t frequency) {
    uint32_t divisor = PIT_FREQ / frequency;
    
    outb(PIT_CMD_PORT, 0x36);
    outb(PIT_CHANNEL_0, divisor & 0xFF);
    outb(PIT_CHANNEL_0, (divisor >> 8) & 0xFF);
    
    timer_ticks = 0;
    register_interrupt_handler(32, timer_callback);
}

uint32_t Timer::get_ticks() {
    return timer_ticks;
}

void Timer::sleep(uint32_t ms) {
    uint32_t start = timer_ticks;
    while (timer_ticks - start < ms) {
        asm volatile ("hlt");
    }
}

void Timer::set_handler(void (*handler)(void)) {
    user_handler = handler;
}