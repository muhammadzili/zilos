#pragma once
#include "utils/types.hpp"

class HTTP {
public:
    static bool get(const char* url, const char* save_filename);

private:
    static bool parse_url(const char* url, char* host, char* path, uint16_t* port, bool* is_https);
    static int str_len(const char* s);
    static void str_cpy(char* dst, const char* src);
};
