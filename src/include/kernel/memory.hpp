#pragma once

#include "utils/types.hpp"

namespace Memory {

struct Frame {
    bool used;
    uint32_t next;
};

class PMM {
public:
    static const uint32_t FRAME_SIZE = 4096;
    static const uint32_t MAX_FRAMES = 32768;

    static void initialize(uint32_t mem_size);
    static void* allocate_frame();
    static void free_frame(void* frame);
    static uint32_t get_total_frames();
    static uint32_t get_free_frames();
    static uint32_t get_used_frames();
};

struct BlockHeader {
    uint32_t size;
    bool free;
    BlockHeader* next;
};

class Heap {
public:
    static const uint32_t HEAP_START = 0xC0000000;
    static const uint32_t HEAP_SIZE = 0x100000;

    static void initialize();
    static void* allocate(uint32_t size);
    static void free(void* ptr);
    static uint32_t get_used();
    static uint32_t get_free();
};

}

void* operator new(uint32_t size);
void operator delete(void* ptr);
void* operator new[](uint32_t size);
void operator delete[](void* ptr);