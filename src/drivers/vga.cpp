#include "drivers/vga.hpp"
#include "utils/ports.hpp"

size_t VGA::row = 0;
size_t VGA::column = 0;
uint8_t VGA::color = 0;
uint16_t* VGA::buffer = nullptr;

void VGA::initialize() {
    row = 0;
    column = 0;
    color = entry_color(15, 0); // White on black
    buffer = (uint16_t*) 0xB8000;
    clear();
    enable_cursor(14, 15);
}

void VGA::clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            buffer[index] = ' ' | (color << 8);
        }
    }
    row = 0;
    column = 0;
    update_cursor(0, 0);
}

void VGA::set_color(uint8_t new_color) {
    color = new_color;
}

uint8_t VGA::entry_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

void VGA::scroll() {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            buffer[y * VGA_WIDTH + x] = buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = ' ' | (color << 8);
    }
    row = VGA_HEIGHT - 1;
}

void VGA::putchar(char c) {
    if (c == '\n') {
        column = 0;
        if (++row == VGA_HEIGHT) {
            scroll();
        }
    } else if (c == '\r') {
        column = 0;
    } else if (c == '\b') {
        if (column > 0) {
            column--;
            buffer[row * VGA_WIDTH + column] = ' ' | (color << 8);
        } else if (row > 0) {
            row--;
            column = VGA_WIDTH - 1;
            buffer[row * VGA_WIDTH + column] = ' ' | (color << 8);
        }
    } else {
        buffer[row * VGA_WIDTH + column] = c | (color << 8);
        if (++column == VGA_WIDTH) {
            column = 0;
            if (++row == VGA_HEIGHT) {
                scroll();
            }
        }
    }
    update_cursor(column, row);
}

void VGA::print(const char* str) {
    while (*str) {
        putchar(*str++);
    }
}

void VGA::println(const char* str) {
    print(str);
    putchar('\n');
}

void VGA::set_cursor(int x, int y) {
    column = x;
    row = y;
    update_cursor(x, y);
}

void VGA::enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void VGA::disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20); // Bit 5 set to 1 disables the cursor
}

void VGA::update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void VGA::itoa(size_t val, char* buf, int base) {
    char* p = buf;
    char* p1 = buf;
    char tmp;
    size_t tv;
    if (val == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }
    do {
        tv = val;
        val /= base;
        *p++ = "0123456789abcdef"[tv - val * base];
    } while (val);
    *p-- = '\0';
    while (p1 < p) {
        tmp = *p;
        *p-- = *p1;
        *p1++ = tmp;
    }
}
