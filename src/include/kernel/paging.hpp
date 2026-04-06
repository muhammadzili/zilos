#pragma once

#include "utils/types.hpp"

namespace Paging {

const uint32_t PAGE_SIZE = 4096;
const uint32_t PAGE_DIR_ENTRIES = 1024;
const uint32_t PAGE_TABLE_ENTRIES = 1024;

struct PageDirectory {
    uint32_t entries[PAGE_DIR_ENTRIES];
} __attribute__((aligned(4096)));

struct PageTable {
    uint32_t entries[PAGE_TABLE_ENTRIES];
} __attribute__((aligned(4096)));

class VM {
public:
    static PageDirectory* kernel_dir;
    static PageTable* kernel_tables;

    static void initialize();
    static void map_page(uint32_t virt, uint32_t phys, uint32_t flags);
    static void unmap_page(uint32_t virt);
    static uint32_t* get_page(uint32_t virt, bool make);
    static void enable_paging();
};

}

extern "C" void enable_paging();