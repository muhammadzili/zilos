#include "drivers/input/keyboard.hpp"
#include "utils/ports.hpp"
#include "drivers/video/vga.hpp"

// --- State Variables ---
bool Keyboard::is_shift_pressed = false;
char Keyboard::last_char = 0;

// special key codes for shell
const char KEY_UP    = 0x80;
const char KEY_DOWN  = 0x81;
const char KEY_LEFT  = 0x82;
const char KEY_RIGHT = 0x83;
const char KEY_PAGE_UP   = 0x84;
const char KEY_PAGE_DOWN = 0x85;

/**
 * @brief Interrupt Handler for Keyboard (IRQ 1)
 */
void Keyboard::handler(registers* r) {
    (void)r;
    
    if (inb(0x64) & 0x01) {
        uint8_t scancode = inb(0x60);
        last_char = scancode_to_ascii(scancode);
    }
}

// --- US QWERTY Keymaps ---

const char kbd_US[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',
    0,  ' ',
    0,
};

const char kbd_US_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,
  '*',
    0,  ' ',
    0,
};

/**
 * @brief Initialize the 8042 PS/2 controller and register handler
 */
void Keyboard::initialize() {
    is_shift_pressed = false;
    last_char = 0;

    while (inb(0x64) & 0x01) {
        inb(0x60);
    }

    register_interrupt_handler(33, Keyboard::handler);
}

/**
 * @brief Translate raw scancodes to ASCII characters
 */
char Keyboard::scancode_to_ascii(uint8_t scancode) {
    if (scancode & 0x80) {
        uint8_t release_code = scancode & 0x7F;
        if (release_code == 0x2A || release_code == 0x36) {
            is_shift_pressed = false;
        }
        if (release_code == 0x48 || release_code == 0x50 || 
            release_code == 0x4B || release_code == 0x4D ||
            release_code == 0x49 || release_code == 0x51) {
            return 0;
        }
    } else {
        if (scancode == 0x2A || scancode == 0x36) {
            is_shift_pressed = true;
            return 0;
        }

        if (scancode == 0x48) return KEY_UP;
        if (scancode == 0x50) return KEY_DOWN;
        if (scancode == 0x4B) return KEY_LEFT;
        if (scancode == 0x4D) return KEY_RIGHT;
        if (scancode == 0x49) return KEY_PAGE_UP;
        if (scancode == 0x51) return KEY_PAGE_DOWN;
        
        if (scancode == 0x3C) return (char)0x84;

        if (scancode < 128) {
            return is_shift_pressed ? kbd_US_shift[scancode] : kbd_US[scancode];
        }
    }
    return 0;
}

/**
 * @brief Non-blocking character fetch
 */
char Keyboard::get_char() {
    if (inb(0x64) & 0x01) {
        uint8_t scancode = inb(0x60);
        return scancode_to_ascii(scancode);
    }
    return 0;
}

/**
 * @brief Blocking character fetch
 */
char Keyboard::wait_get_char() {
    while (!(inb(0x64) & 0x01));
    uint8_t scancode = inb(0x60);
    return scancode_to_ascii(scancode);
}

/**
 * @brief Blocking wait for any key event
 */
void Keyboard::wait_for_key() {
    while (!(inb(0x64) & 0x01)) {
        asm volatile("pause");
    }
}
