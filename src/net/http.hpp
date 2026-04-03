#pragma once
#include "utils/types.hpp"

class HTTP {
public:
    // Download a file from url and save to VFS filename
    // url format: "http://hostname/path" or "http://ip/path"
    static bool get(const char* url, const char* save_filename);

private:
    static bool parse_url(const char* url, char* host, char* path, uint16_t* port);
    static int str_len(const char* s);
    static void str_cpy(char* dst, const char* src);
};
