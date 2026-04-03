#pragma once
#include "utils/types.hpp"

class Commands {
public:
    static void execute(const char* cmd, const char* args);
    static void cmd_net(const char* args);
    static void cmd_ifconfig();
    
private:
    static bool strcmp(const char* s1, const char* s2);
    static void cmd_help();
    static void cmd_clear();
    static void cmd_echo(const char* args);
    static void cmd_about();
    
    // File system commands
    static void cmd_mkdir(const char* args);
    static void cmd_ls();
    static void cmd_install();
    static void cmd_save();
    static void cmd_cat(const char* args);
    static void cmd_rm(const char* args);
    static void cmd_edit(const char* args);
    
    // System commands
    static void cmd_cpu();
    static void cmd_memory();
    static void cmd_storage();
    static void cmd_date();
    static void cmd_whoami();
    static void cmd_uname();
    static void cmd_pwd();
    static void cmd_touch(const char* args);
    // New Commands
    static void cmd_hostname();
    static void cmd_uptime();
    static void cmd_sleep_cmd(const char* args);
    static void cmd_dmesg();

    // Network commands
    static void cmd_ping(const char* args);
    static void cmd_dns(const char* args);
    static void cmd_wget(const char* args);
    static void cmd_reboot();
    static void cmd_shutdown();
    
    // Helpers
    static void itoa(size_t value, char* str, int base);
    static void sleep(uint32_t ms);
};
