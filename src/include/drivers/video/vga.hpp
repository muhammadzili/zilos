#pragma once

#include "utils/types.hpp"

/**
 * @brief VGA Hardware Driver
 * 
 * Provides an interface for the 80x25 text mode VGA buffer located at 0xB8000.
 * Handles character output, scrolling, and hardware cursor manipulation.
 */
class VGA {
public:
    /**
     * @brief Screen Dimensions
     */
    static const size_t VGA_WIDTH = 80;
    static const size_t VGA_HEIGHT = 25;
    static const size_t SCROLLBACK_LINES = 100;

    // --- Initialization & Control ---
    static void initialize();
    static void clear();
    static void set_color(uint8_t color);
    static uint8_t entry_color(uint8_t fg, uint8_t bg);

    // --- Output Operations ---
    static void putchar(char c);
    static void print(const char* str);
    static void println(const char* str);
    static void itoa(size_t val, char* buf, int base);

    // --- Scrollback Operations ---
    static void scroll_up();
    static void scroll_down();
    static bool is_scrolled_up();
    static void scroll_to_bottom();
    static void scroll_page_up();
    static void scroll_page_down();
    static void render_scrollback();

    // --- Cursor Manipulation ---
    static void set_cursor(int x, int y);
    static void disable_cursor();
    static void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
    static void update_cursor(int x, int y);

private:
    static size_t row;
    static size_t column;
    static uint8_t color;
    static uint16_t* buffer;
    
    // scrollback buffer
    static uint16_t scrollback[SCROLLBACK_LINES][VGA_WIDTH];
    static size_t scrollback_start;
    static size_t scrollback_count;
    static size_t scroll_offset;
    static bool in_scrollback;
    
    /**
     * @brief Scroll the terminal buffer up by one line
     */
    static void scroll();
    static void add_to_scrollback();
};

/**
 * @brief Standard VGA 16-color palette
 */
enum VGA_COLOR {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15,
};
