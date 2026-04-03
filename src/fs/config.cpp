#include "fs/config.hpp"
#include "fs/vfs.hpp"

SystemConfig Config::config;

static void config_strcpy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

static bool config_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) { a++; b++; }
    return *a == *b;
}

void Config::init() {
    config.installed = false;
    config.gui_enabled = false;
    config.username[0] = '\0';
    load();
}

bool Config::is_installed() { return config.installed; }
bool Config::is_gui_enabled() { return config.gui_enabled; }
const char* Config::get_username() { return config.username; }

void Config::set_installed(bool val) { config.installed = val; }
void Config::set_gui_enabled(bool val) { config.gui_enabled = val; }
void Config::set_username(const char* name) {
    int i = 0;
    while (name[i] && i < MAX_USERNAME - 1) {
        config.username[i] = name[i];
        i++;
    }
    config.username[i] = '\0';
}

bool Config::save() {
    // Store config as simple key=value format
    VFile* f = VFS::get_file(CONFIG_FILENAME);
    if (!f) {
        VFS::create_file(CONFIG_FILENAME, false);
    }
    
    char buf[256];
    int p = 0;
    
    // installed=1\n
    const char* k1 = "installed=";
    for (int i = 0; k1[i]; i++) buf[p++] = k1[i];
    buf[p++] = config.installed ? '1' : '0';
    buf[p++] = '\n';
    
    // gui=1\n
    const char* k2 = "gui=";
    for (int i = 0; k2[i]; i++) buf[p++] = k2[i];
    buf[p++] = config.gui_enabled ? '1' : '0';
    buf[p++] = '\n';
    
    // user=username\n
    const char* k3 = "user=";
    for (int i = 0; k3[i]; i++) buf[p++] = k3[i];
    for (int i = 0; config.username[i]; i++) buf[p++] = config.username[i];
    buf[p++] = '\n';
    
    buf[p] = '\0';
    
    return VFS::write_file(CONFIG_FILENAME, buf);
}

bool Config::load() {
    VFile* f = VFS::get_file(CONFIG_FILENAME);
    if (!f) return false;
    
    const char* data = f->content;
    int i = 0;
    
    while (data[i]) {
        // Parse key=value pairs
        char key[32] = {0};
        char val[32] = {0};
        int ki = 0, vi = 0;
        
        // Read key
        while (data[i] && data[i] != '=' && data[i] != '\n') {
            if (ki < 31) key[ki++] = data[i];
            i++;
        }
        key[ki] = '\0';
        
        if (data[i] == '=') {
            i++; // skip '='
            // Read value
            while (data[i] && data[i] != '\n') {
                if (vi < 31) val[vi++] = data[i];
                i++;
            }
            val[vi] = '\0';
        }
        
        if (data[i] == '\n') i++;
        
        // Apply
        if (config_strcmp(key, "installed")) {
            config.installed = (val[0] == '1');
        } else if (config_strcmp(key, "gui")) {
            config.gui_enabled = (val[0] == '1');
        } else if (config_strcmp(key, "user")) {
            config_strcpy(config.username, val);
        }
    }
    
    return true;
}
