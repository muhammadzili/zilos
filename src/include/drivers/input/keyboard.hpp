#pragma once
#include "utils/types.hpp"

#include "arch/i386/interrupts.hpp"

class Keyboard {
public:
    static void initialize();
    static void handler(registers* r);
    static char get_char();
    static char wait_get_char();
    static void wait_for_key();
private:
    static char scancode_to_ascii(uint8_t scancode);
    static bool is_shift_pressed;
    static char last_char;
};
