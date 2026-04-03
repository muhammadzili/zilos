#include "cli/shell.hpp"
#include "drivers/vga.hpp"
#include "drivers/keyboard.hpp"
#include "cli/commands.hpp"

char Shell::buffer[BUFFER_SIZE];
size_t Shell::buffer_pos = 0;
const char* Shell::prompt = "zilos> ";
bool Shell::gui_mode = false;
bool Shell::should_exit = false;
char Shell::prompt_buf[64];

static bool shell_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *a == *b;
}

void Shell::init() {
    gui_mode = false;
    should_exit = false;
    buffer_pos = 0;
    prompt = "zilos> ";
    
    // Enforce pure black & white as requested
    VGA::set_color(VGA::entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    VGA::clear();
    VGA::println("======================================");
    VGA::println("          Welcome to ZilOS!           ");
    VGA::println("======================================");
    VGA::println("Type 'help' for a list of commands.");
    print_prompt();
}

void Shell::init_gui_mode(const char* username) {
    gui_mode = true;
    should_exit = false;
    buffer_pos = 0;
    
    // Build prompt: "username@zilos> "
    int i = 0;
    for (int j = 0; username[j] && i < 50; j++) {
        prompt_buf[i++] = username[j];
    }
    const char* suffix = "@zilos> ";
    for (int j = 0; suffix[j] && i < 62; j++) {
        prompt_buf[i++] = suffix[j];
    }
    prompt_buf[i] = '\0';
    prompt = prompt_buf;
    
    print_prompt();
}

void Shell::print_prompt() {
    VGA::print(prompt);
}

void Shell::execute_command() {
    buffer[buffer_pos] = '\0';

    if (buffer_pos == 0) {
        print_prompt();
        return;
    }

    // Split command and arguments
    char cmd[BUFFER_SIZE];
    char args[BUFFER_SIZE];
    cmd[0] = '\0';
    args[0] = '\0';

    size_t i = 0;
    while (i < buffer_pos && buffer[i] != ' ') {
        cmd[i] = buffer[i];
        i++;
    }
    cmd[i] = '\0';

    if (buffer[i] == ' ') {
        i++;
        size_t j = 0;
        while (i < buffer_pos) {
            args[j] = buffer[i];
            i++; j++;
        }
        args[j] = '\0';
    }

    // Check for exit command (GUI mode only)
    if (shell_strcmp(cmd, "exit")) {
        if (gui_mode) {
            should_exit = true;
            buffer_pos = 0;
            return;
        } else {
            VGA::println("Exit is only available in GUI terminal mode.");
        }
    } else {
        Commands::execute(cmd, args);
    }

    buffer_pos = 0;
    print_prompt();
}

void Shell::handle_input(char c) {
    if (c == 0) return;

    if (c == '\b') {
        if (buffer_pos > 0) {
            buffer_pos--;
            VGA::putchar('\b');
        }
    } else if (c == '\n') {
        VGA::putchar('\n');
        execute_command();
    } else {
        if (buffer_pos < BUFFER_SIZE - 1) {
            buffer[buffer_pos++] = c;
            VGA::putchar(c);
        }
    }
}

void Shell::run() {
    while (!should_exit) {
        char c = Keyboard::get_char();
        if (c != 0) {
            handle_input(c);
        }
    }
}
