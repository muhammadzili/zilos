#include "apps/editor.hpp"
#include "utils/types.hpp"
#include "drivers/vga.hpp"
#include "drivers/keyboard.hpp"
#include "fs/vfs.hpp"
#include "net/netutils.hpp"

// VGA Hardware Address
#define VGA_MEM ((uint16_t*)0xB8000)

static char grid[23][80];
static int cur_x = 0;
static int cur_y = 0;

static void zedit_put_hw(int x, int y, char c, uint8_t color) {
    if (x < 0 || x >= 80 || y < 0 || y >= 25) return;
    VGA_MEM[y * 80 + x] = (uint16_t)c | ((uint16_t)color << 8);
}

static void zedit_draw_ui(const char* filename) {
    // Header (Blue)
    uint8_t h_color = VGA::entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    for (int i = 0; i < 80; i++) zedit_put_hw(i, 0, ' ', h_color);
    
    const char* title = "ZEDIT v1.4 [PRO]";
    for(int i=0; title[i]; i++) zedit_put_hw(2+i, 0, title[i], h_color);
    
    int fn_len = 0; while(filename[fn_len]) fn_len++;
    for(int i=0; i<fn_len; i++) zedit_put_hw(40-(fn_len/2)+i, 0, filename[i], h_color);
    
    // Footer (Grey)
    uint8_t f_color = VGA::entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREY);
    for (int i = 0; i < 80; i++) zedit_put_hw(i, 24, ' ', f_color);
    const char* help = "ARROWS: Navigation  |  BACKSPACE: Delete  |  ESC: Save & Exit";
    for(int i=0; help[i]; i++) zedit_put_hw(2+i, 24, help[i], f_color);
}

static void zedit_refresh_grid() {
    uint8_t color = VGA::entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    for (int y = 0; y < 23; y++) {
        for (int x = 0; x < 80; x++) {
            zedit_put_hw(x, y + 1, grid[y][x] ? grid[y][x] : ' ', color);
        }
    }
}

void Editor::start(const char* filename) {
    // Clear screen first
    for(int i=0; i<80*25; i++) VGA_MEM[i] = (uint16_t)' ' | (0x07 << 8);
    
    net_memset(grid, 0, sizeof(grid));
    cur_x = 0; cur_y = 0;

    VFile* f = VFS::get_file(filename);
    if (f && !f->is_dir) {
        int tx = 0, ty = 0;
        for (size_t i = 0; i < f->size && ty < 23; i++) {
            char c = f->content[i];
            if (c == '\n') { tx = 0; ty++; }
            else { 
                grid[ty][tx++] = c; 
                if (tx >= 80) { tx = 0; ty++; }
            }
        }
    }

    zedit_draw_ui(filename);
    zedit_refresh_grid();

    while (true) {
        // Update hardware cursor
        VGA::set_cursor(cur_x, cur_y + 1);
        
        char c = Keyboard::wait_get_char(); // BLOCKING
        if (c == 0) continue;
        if (c == 27) break; // ESC

        if (c == (char)0x80) { // UP
            if (cur_y > 0) cur_y--;
        } else if (c == (char)0x81) { // DOWN
            if (cur_y < 22) cur_y++;
        } else if (c == (char)0x82) { // LEFT
            if (cur_x > 0) cur_x--;
            else if (cur_y > 0) { cur_x = 79; cur_y--; }
        } else if (c == (char)0x83) { // RIGHT
            if (cur_x < 79) cur_x++;
            else if (cur_y < 22) { cur_x = 0; cur_y++; }
        } else if (c == '\b') {
            if (cur_x > 0) {
                cur_x--;
                grid[cur_y][cur_x] = ' ';
            } else if (cur_y > 0) {
                cur_y--; cur_x = 79;
                grid[cur_y][cur_x] = ' ';
            }
            zedit_refresh_grid();
        } else if (c == '\n') {
            if (cur_y < 22) { cur_y++; cur_x = 0; }
        } else if (c >= 32 && c <= 126) {
            grid[cur_y][cur_x] = c;
            zedit_put_hw(cur_x, cur_y + 1, c, 0x07);
            cur_x++;
            if (cur_x >= 80) { cur_x = 0; cur_y++; }
            if (cur_y > 22) { cur_y = 22; cur_x = 79; }
        }
    }

    // Save
    char out_buf[MAX_FILESIZE];
    net_memset(out_buf, 0, MAX_FILESIZE);
    size_t p = 0;
    for (int y = 0; y < 23; y++) {
        int last = -1;
        for (int x = 0; x < 80; x++) if (grid[y][x] && grid[y][x] != ' ') last = x;
        for (int x = 0; x <= last && p < MAX_FILESIZE - 2; x++) {
            out_buf[p++] = grid[y][x] ? grid[y][x] : ' ';
        }
        if (p < MAX_FILESIZE - 2) out_buf[p++] = '\n';
    }

    if (!f) VFS::create_file(filename, false);
    VFS::write_file(filename, out_buf);
    VFS::save_to_disk();

    VGA::clear();
}
