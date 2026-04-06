#include "fs/vfs.hpp"
#include "hal/hal.hpp"

// static storage for in-memory file table
VFile VFS::files[MAX_FILES];

size_t VFS::strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

bool VFS::strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

void VFS::strcpy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void VFS::init() {
    load_from_disk();
}

#define VFS_MAGIC 0x5A494C4F
#define VFS_LBA_START 100 // start sector for vfs metadata

struct VFSDiskHeader {
    uint32_t magic;
    uint32_t num_files;
};

// save_to_disk: dump the entire file table via hal fr
bool VFS::save_to_disk() {
    BlockDevice* dev = HAL::get_block_device();
    if (!dev) return false;

    VFSDiskHeader header;
    header.magic = VFS_MAGIC;
    header.num_files = MAX_FILES;
    
    uint8_t header_sector[512] = {0};
    uint8_t* hp = (uint8_t*)&header;
    for(size_t i=0; i<sizeof(VFSDiskHeader); i++) header_sector[i] = hp[i];
    
    // write header first idc if it takes time
    if (!dev->write_sectors((uint32_t)header_sector, VFS_LBA_START, 1)) return false;
    
    uint32_t data_size = sizeof(files);
    uint32_t sectors_needed = (data_size / 512) + 1;
    // push all file data to sectors after header fr
    if (!dev->write_sectors((uint32_t)files, VFS_LBA_START + 1, sectors_needed)) return false;
    
    return true;
}

// load_from_disk: check for vfs magic and load files via hal fr
bool VFS::load_from_disk() {
    BlockDevice* dev = HAL::get_block_device();
    if (!dev) return false;

    uint8_t header_sector[512] = {0};
    if (!dev->read_sectors((uint32_t)header_sector, VFS_LBA_START, 1)) {
        // failed to read; init empty idc
        for (int i = 0; i < MAX_FILES; i++) {
            files[i].exists = false;
            files[i].is_dir = false;
            files[i].size = 0;
            files[i].name[0] = '\0';
        }
        return false;
    }
    
    VFSDiskHeader* header = (VFSDiskHeader*)header_sector;
    if (header->magic == VFS_MAGIC) {
        uint32_t data_size = sizeof(files);
        uint32_t sectors_needed = (data_size / 512) + 1;
        // magic matches; load all sectors fr
        if (!dev->read_sectors((uint32_t)files, VFS_LBA_START + 1, sectors_needed)) return false;
        return true;
    } else {
        // fresh drive or wrong magic; setup empty ig
        for (int i = 0; i < MAX_FILES; i++) {
            files[i].exists = false;
            files[i].is_dir = false;
            files[i].size = 0;
            files[i].name[0] = '\0';
        }
        return true;
    }
}

// create_file: find empty slot and setup new file/dir
bool VFS::create_file(const char* name, bool is_dir) {
    if (get_file(name) != NULL) {
        return false; // already exists; ts is not allowed
    }
    
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].exists) {
            files[i].exists = true;
            files[i].is_dir = is_dir;
            files[i].size = 0;
            strcpy(files[i].name, name);
            files[i].content[0] = '\0';
            return true;
        }
    }
    return false; // no space left lol
}

bool VFS::delete_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].exists && strcmp(files[i].name, name)) {
            files[i].exists = false; // just mark as dead idc about wiping
            return true;
        }
    }
    return false;
}

VFile* VFS::get_file(const char* name) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].exists && strcmp(files[i].name, name)) {
            return &files[i];
        }
    }
    return NULL;
}

VFile* VFS::get_all_files() {
    return files;
}

// write_file: update content of existing file fr
bool VFS::write_file(const char* name, const char* content) {
    VFile* f = get_file(name);
    if (!f || f->is_dir) return false;
    
    size_t len = strlen(content);
    if (len >= MAX_FILESIZE) len = MAX_FILESIZE - 1; // truncate if too big ngl
    
    for (size_t i = 0; i < len; i++) {
        f->content[i] = content[i];
    }
    f->content[len] = '\0';
    f->size = len;
    return true;
}

// get_used_storage: calc total bytes used including metadata
size_t VFS::get_used_storage() {
    size_t total = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].exists) {
            total += files[i].size + sizeof(VFile); 
        }
    }
    return total;
}
