#pragma once
#include "utils/types.hpp"

class Shell {
public:
    static void init();
    static void init_gui_mode(const char* username);
    static void run();
private:
    static const size_t BUFFER_SIZE = 256;
    static char buffer[BUFFER_SIZE];
    static size_t buffer_pos;

    static void print_prompt();
    static void handle_input(char c);
    static void execute_command();
    
    static const char* prompt;
    static bool gui_mode;
    static bool should_exit;
    static char prompt_buf[64];
};
