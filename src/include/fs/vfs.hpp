#pragma once
#include "utils/types.hpp"

#define MAX_FILES 32
#define MAX_FILENAME 32
#define MAX_FILESIZE 4096

struct VFile {
    bool exists;
    bool is_dir;
    char name[MAX_FILENAME];
    char content[MAX_FILESIZE];
    size_t size;
};

class VFS {
public:
    static void init();
    static bool create_file(const char* name, bool is_dir);
    static bool delete_file(const char* name);
    static VFile* get_file(const char* name);
    static VFile* get_all_files();
    static bool load_from_disk();
    static bool save_to_disk();
    static bool write_file(const char* name, const char* content);
    static size_t get_used_storage();
    
private:
    static VFile files[MAX_FILES];
    static bool strcmp(const char* s1, const char* s2);
    static void strcpy(char* dest, const char* src);
    static size_t strlen(const char* str);
};
