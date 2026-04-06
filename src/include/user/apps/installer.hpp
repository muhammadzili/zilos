#pragma once
#include "utils/types.hpp"

class Installer {
public:
    static void run();
    
private:
    static void draw_box(int x, int y, int w, int h, uint8_t bg);
    static void draw_text_centered(int y, const char* text, uint8_t fg, uint8_t bg);
    static void draw_text(int x, int y, const char* text, uint8_t fg, uint8_t bg);
    static void draw_progress_bar(int y, int percent);
    static void screen_step(const char* title, int start_pct, int end_pct, int delay_ms, const char* files[]);
    
    static void screen_welcome();
    static void screen_sysinfo();
    static void screen_network();
    static void screen_username();
    static void screen_installing();
    static void screen_complete();
    
    static void itoa(size_t val, char* buf, int base);
    static int strlen(const char* s);
};
