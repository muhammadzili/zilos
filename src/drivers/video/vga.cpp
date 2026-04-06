#include "drivers/video/vga.hpp"
#include "utils/ports.hpp"

// static members init
size_t VGA::row = 0;
size_t VGA::column = 0;
uint8_t VGA::color = 0;
uint16_t* VGA::buffer = nullptr;

// scrollback
uint16_t VGA::scrollback[SCROLLBACK_LINES][VGA_WIDTH] = {0};
size_t VGA::scrollback_start = 0;
size_t VGA::scrollback_count = 0;
size_t VGA::scroll_offset = 0;
bool VGA::in_scrollback = false;

// init vga: setup initial state and clear screen fr
void VGA::initialize() {
    row = 0;
    column = 0;
    color = entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    buffer = (uint16_t*) 0xB8000;
    scrollback_start = 0;
    scrollback_count = 0;
    scroll_offset = 0;
    in_scrollback = false;
    clear();
    enable_cursor(14, 15);
}

// clear: wipe screen with current bg color
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

// scroll: shift everything up by 1 line lol
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

void VGA::add_to_scrollback() {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        scrollback[scrollback_start][x] = buffer[row * VGA_WIDTH + x];
    }
    scrollback_start = (scrollback_start + 1) % SCROLLBACK_LINES;
    if (scrollback_count < SCROLLBACK_LINES) {
        scrollback_count++;
    }
}

void VGA::scroll_to_bottom() {
    if (scroll_offset > 0) {
        scroll_offset = 0;
        in_scrollback = false;
        for (size_t y = 0; y < VGA_HEIGHT; y++) {
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                buffer[y * VGA_WIDTH + x] = ' ' | (color << 8);
            }
        }
        row = 0;
        column = 0;
        update_cursor(0, 0);
    }
}

bool VGA::is_scrolled_up() {
    return in_scrollback;
}

void VGA::scroll_up() {
    if (!in_scrollback && scrollback_count > 0) {
        in_scrollback = true;
    }
    
    if (in_scrollback && scroll_offset < scrollback_count - VGA_HEIGHT) {
        scroll_offset++;
    }
    
    render_scrollback();
}

void VGA::scroll_down() {
    if (in_scrollback) {
        if (scroll_offset > 0) {
            scroll_offset--;
        }
        if (scroll_offset == 0) {
            scroll_to_bottom();
            return;
        }
        render_scrollback();
    }
}

void VGA::scroll_page_up() {
    for (int i = 0; i < VGA_HEIGHT - 1; i++) {
        scroll_up();
    }
}

void VGA::scroll_page_down() {
    for (int i = 0; i < VGA_HEIGHT - 1; i++) {
        scroll_down();
    }
}

void VGA::render_scrollback() {
    if (!in_scrollback) return;
    
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        size_t line_idx = (scrollback_start + scroll_offset + y) % SCROLLBACK_LINES;
        if (scroll_offset + y >= scrollback_count) break;
        
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            buffer[y * VGA_WIDTH + x] = scrollback[line_idx][x];
        }
    }
    
    update_cursor(column, row);
}

// putchar: print single char; handle newline and stuff
void VGA::putchar(char c) {
    if (in_scrollback && c != 0) {
        scroll_to_bottom();
    }
    
    if (c == '\n') {
        column = 0;
        if (++row == VGA_HEIGHT) {
            add_to_scrollback();
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
                add_to_scrollback();
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

// enable_cursor: setup hardware cursor shape
void VGA::enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

// disable_cursor: hide it idc
void VGA::disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

// update_cursor: move it to (x, y)
void VGA::update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

// itoa: int to string conv; basic but works ngl
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