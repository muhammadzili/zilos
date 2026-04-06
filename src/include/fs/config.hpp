#pragma once
#include "utils/types.hpp"

#define CONFIG_FILENAME ".zilos_config"
#define MAX_USERNAME 24

struct SystemConfig {
    bool installed;
    bool gui_enabled;
    char username[MAX_USERNAME];
};

class Config {
public:
    static void init();
    static bool is_installed();
    static bool is_gui_enabled();
    static const char* get_username();
    
    static void set_installed(bool val);
    static void set_gui_enabled(bool val);
    static void set_username(const char* name);
    
    static bool save();
    static bool load();

private:
    static SystemConfig config;
};
