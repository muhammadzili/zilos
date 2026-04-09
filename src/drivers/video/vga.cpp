#include "drivers/video/vga.hpp"
#include "utils/ports.hpp"

// static members init
size_t VGA::row = 0;
size_t VGA::column = 0;
uint8_t VGA::color = 0;
uint16_t* VGA::buffer = nullptr;

// scrollback - use separate display buffer
uint16_t VGA::scrollback[SCROLLBACK_LINES][VGA_HEIGHT][VGA_WIDTH] = {0};
size_t VGA::scrollback_count = 0;
size_t VGA::scroll_offset = 0;
bool VGA::in_scrollback = false;

// saved screen when scrolled up
uint16_t VGA::saved_screen[VGA_HEIGHT][VGA_WIDTH] = {0};
size_t VGA::saved_row = 0;
size_t VGA::saved_column = 0;

// init vga
void VGA::initialize() {
    row = 0;
    column = 0;
    color = entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    buffer = (uint16_t*) 0xB8000;
    scrollback_count = 0;
    scroll_offset = 0;
    in_scrollback = false;
    saved_row = 0;
    saved_column = 0;
    clear();
    enable_cursor(14, 15);
}

// clear screen
void VGA::clear() {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            buffer[index] = ' ' | (color << 8);
        }
    }
    row = 0;
    column = 0;
    scroll_offset = 0;
    in_scrollback = false;
    update_cursor(0, 0);
}

void VGA::set_color(uint8_t new_color) {
    color = new_color;
}

uint8_t VGA::entry_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

// scroll - shift up and save all lines to scrollback
void VGA::scroll() {
    if (in_scrollback) {
        scroll_to_bottom();
    }
    
    if (scrollback_count < SCROLLBACK_LINES) {
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                scrollback[scrollback_count][y][x] = buffer[y * VGA_WIDTH + x];
            }
        }
        scrollback_count++;
    } else {
        for (size_t i = 0; i < SCROLLBACK_LINES - 1; i++) {
            for (size_t y = 0; y < VGA_HEIGHT; y++) {
                for (size_t x = 0; x < VGA_WIDTH; x++) {
                    scrollback[i][y][x] = scrollback[i+1][y][x];
                }
            }
        }
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                scrollback[SCROLLBACK_LINES-1][y][x] = buffer[y * VGA_WIDTH + x];
            }
        }
    }
    
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

void VGA::scroll_to_bottom() {
    if (!in_scrollback) return;
    
    in_scrollback = false;
    scroll_offset = 0;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            buffer[y * VGA_WIDTH + x] = saved_screen[y][x];
        }
    }
    row = saved_row;
    column = saved_column;
    update_cursor(column, row);
}

void VGA::scroll_up() {
    if (scrollback_count == 0) return;
    
    if (!in_scrollback) {
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                saved_screen[y][x] = buffer[y * VGA_WIDTH + x];
            }
        }
        saved_row = row;
        saved_column = column;
        in_scrollback = true;
    }
    
    if (scroll_offset < scrollback_count - 1) {
        scroll_offset++;
        render_scrollback();
    }
}

void VGA::scroll_down() {
    if (!in_scrollback) return;
    
    if (scroll_offset > 0) {
        scroll_offset--;
        render_scrollback();
    }
    
    if (scroll_offset == 0) {
        scroll_to_bottom();
    }
}

void VGA::scroll_page_up() {
    for (size_t i = 0; i < VGA_HEIGHT - 1; i++) {
        scroll_up();
    }
}

void VGA::scroll_page_down() {
    for (size_t i = 0; i < VGA_HEIGHT - 1; i++) {
        scroll_down();
    }
}

void VGA::render_scrollback() {
    if (scroll_offset == 0) {
        scroll_to_bottom();
        return;
    }
    
    size_t start_idx = scroll_offset - 1;
    size_t screen_line = 0;
    
    for (size_t i = start_idx; i < scrollback_count && screen_line < VGA_HEIGHT; i++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            buffer[screen_line * VGA_WIDTH + x] = scrollback[i][screen_line][x];
        }
        screen_line++;
    }
    
    for (size_t y = screen_line; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            buffer[y * VGA_WIDTH + x] = ' ' | (color << 8);
        }
    }
    
    update_cursor(0, VGA_HEIGHT - 1);
}

bool VGA::is_scrolled_up() {
    return in_scrollback && scroll_offset > 0;
}

// putchar
void VGA::putchar(char c) {
    if (in_scrollback && c != 0) {
        scroll_to_bottom();
    }
    
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

void VGA::print_hex(uint32_t val) {
    const char* hex = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        putchar(hex[(val >> (i * 4)) & 0xF]);
    }
}

void VGA::println_hex(uint32_t val) {
    print_hex(val);
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
    outb(0x3D5, 0x20);
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