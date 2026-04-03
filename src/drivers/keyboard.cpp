#include "drivers/keyboard.hpp"
#include "utils/ports.hpp"

bool Keyboard::is_shift_pressed = false;
char Keyboard::last_char = 0;

void Keyboard::handler(registers* r) {
    (void)r;
    if (inb(0x64) & 1) {
        uint8_t scancode = inb(0x60);
        last_char = scancode_to_ascii(scancode);
    }
}

// Simple US QWERTY Map
const char kbd_US[128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',
    0,  ' ',
    0,
};

const char kbd_US_shift[128] =
{
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,
  '*',
    0,  ' ',
    0,
};

void Keyboard::initialize() {
    is_shift_pressed = false;
    last_char = 0;
    // Clear buffer
    while (inb(0x64) & 1) {
        inb(0x60);
    }
    register_interrupt_handler(33, Keyboard::handler); // IRQ 1
}

char Keyboard::scancode_to_ascii(uint8_t scancode) {
    if (scancode & 0x80) {
        // Key release
        uint8_t release_code = scancode & 0x7F;
        if (release_code == 0x2A || release_code == 0x36) {
            is_shift_pressed = false;
        }
    } else {
        // Key press
        if (scancode == 0x2A || scancode == 0x36) {
            is_shift_pressed = true;
            return 0;
        }

        // Arrow Keys and Special Keys
        if (scancode == 0x48) return (char)0x80; // Up
        if (scancode == 0x50) return (char)0x81; // Down
        if (scancode == 0x4B) return (char)0x82; // Left
        if (scancode == 0x4D) return (char)0x83; // Right
        if (scancode == 0x3C) return (char)0x84; // F2 (Prettier)

        if (scancode < 128) {
            if (is_shift_pressed) {
                return kbd_US_shift[scancode];
            } else {
                return kbd_US[scancode];
            }
        }
    }
    return 0;
}

char Keyboard::get_char() {
    if (inb(0x64) & 1) {
        uint8_t scancode = inb(0x60);
        return scancode_to_ascii(scancode);
    }
    return 0;
}

char Keyboard::wait_get_char() {
    while (!(inb(0x64) & 1));
    uint8_t scancode = inb(0x60);
    return scancode_to_ascii(scancode);
}

void Keyboard::wait_for_key() {
    while (!(inb(0x64) & 1)) {
        // busy wait
    }
}
