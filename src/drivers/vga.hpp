#pragma once
#include "utils/types.hpp"

class VGA {
public:
    static void initialize();
    static void clear();
    static void set_color(uint8_t color);
    static uint8_t entry_color(uint8_t fg, uint8_t bg);
    static void putchar(char c);
    static void print(const char* str);
    static void println(const char* str);
    static void set_cursor(int x, int y);
    static void disable_cursor();
    static void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
    static void update_cursor(int x, int y);
    static void itoa(size_t val, char* buf, int base);

private:
    static const size_t VGA_WIDTH = 80;
    static const size_t VGA_HEIGHT = 25;
    static size_t row;
    static size_t column;
    static uint8_t color;
    static uint16_t* buffer;
    
    static void scroll();
};

enum VGA_COLOR {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};
