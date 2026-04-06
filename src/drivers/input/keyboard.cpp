#include "drivers/input/keyboard.hpp"
#include "utils/ports.hpp"

// --- State Variables ---
bool Keyboard::is_shift_pressed = false;
char Keyboard::last_char = 0;

/**
 * @brief Interrupt Handler for Keyboard (IRQ 1)
 */
void Keyboard::handler(registers* r) {
    (void)r; // Unused parameter
    
    // Check if the keyboard output buffer is full
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

    // Flush any pending data in the output buffer
    while (inb(0x64) & 0x01) {
        inb(0x60);
    }

    // Register with Exception/Interrupt Manager (IRQ 1)
    register_interrupt_handler(33, Keyboard::handler);
}

/**
 * @brief Translate raw scancodes to ASCII characters
 */
char Keyboard::scancode_to_ascii(uint8_t scancode) {
    if (scancode & 0x80) {
        // Key Release
        uint8_t release_code = scancode & 0x7F;
        if (release_code == 0x2A || release_code == 0x36) {
            is_shift_pressed = false;
        }
    } else {
        // Key Press
        if (scancode == 0x2A || scancode == 0x36) {
            is_shift_pressed = true;
            return 0;
        }

        // --- Special Key Mapping ---
        if (scancode == 0x48) return (char)0x80; // Up Arrow
        if (scancode == 0x50) return (char)0x81; // Down Arrow
        if (scancode == 0x4B) return (char)0x82; // Left Arrow
        if (scancode == 0x4D) return (char)0x83; // Right Arrow
        if (scancode == 0x3C) return (char)0x84; // F2 Function

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
        asm volatile("pause"); // CPU optimization for busy loops
    }
}
