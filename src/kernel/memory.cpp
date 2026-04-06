#include "kernel/memory.hpp"
#include "drivers/video/vga.hpp"

namespace Memory {

static Frame frames[PMM::MAX_FRAMES];
static uint32_t total_frames = 0;
static uint32_t free_frames = 0;
static uint32_t first_free = 0;

void PMM::initialize(uint32_t mem_size) {
    total_frames = mem_size / PMM::FRAME_SIZE;
    if (total_frames > PMM::MAX_FRAMES) total_frames = PMM::MAX_FRAMES;

    for (uint32_t i = 0; i < total_frames; i++) {
        frames[i].used = false;
        frames[i].next = i + 1;
    }
    frames[total_frames - 1].next = 0xFFFFFFFF;
    first_free = 0;
    free_frames = total_frames;
}

void* PMM::allocate_frame() {
    if (first_free == 0xFFFFFFFF || free_frames == 0) return 0;

    uint32_t idx = first_free;
    frames[idx].used = true;
    first_free = frames[idx].next;
    free_frames--;

    return (void*)(idx * PMM::FRAME_SIZE);
}

void PMM::free_frame(void* frame) {
    uint32_t idx = (uint32_t)frame / PMM::FRAME_SIZE;
    if (idx >= total_frames) return;

    frames[idx].used = false;
    frames[idx].next = first_free;
    first_free = idx;
    free_frames++;
}

uint32_t PMM::get_total_frames() { return total_frames; }
uint32_t PMM::get_free_frames() { return free_frames; }
uint32_t PMM::get_used_frames() { return total_frames - free_frames; }

static BlockHeader* heap_start = 0;
static BlockHeader* heap_end = 0;
static uint32_t heap_used = 0;

void Heap::initialize() {
    heap_start = (BlockHeader*)Heap::HEAP_START;
    heap_end = heap_start;
    heap_start->size = 0;
    heap_start->free = false;
    heap_start->next = 0;
    heap_used = sizeof(BlockHeader);
}

void* Heap::allocate(uint32_t size) {
    if (size == 0) return 0;

    BlockHeader* prev = heap_start;
    BlockHeader* curr = heap_start->next;

    while (curr) {
        if (curr->free && curr->size >= size) {
            if (curr->size > size + sizeof(BlockHeader) + 32) {
                BlockHeader* new_block = (BlockHeader*)((uint8_t*)curr + sizeof(BlockHeader) + size);
                new_block->size = curr->size - size - sizeof(BlockHeader);
                new_block->free = true;
                new_block->next = curr->next;
                curr->size = size;
                curr->next = new_block;
            }
            curr->free = false;
            heap_used += size;
            return (void*)((uint8_t*)curr + sizeof(BlockHeader));
        }
        prev = curr;
        curr = curr->next;
    }

    uint32_t needed = size + sizeof(BlockHeader);
    if ((uint32_t)heap_end - Heap::HEAP_START + needed > Heap::HEAP_SIZE) return 0;

    BlockHeader* new_block = heap_end;
    heap_end = (BlockHeader*)((uint8_t*)heap_end + needed);
    new_block->size = size;
    new_block->free = false;
    new_block->next = 0;
    prev->next = new_block;
    heap_used += needed;

    return (void*)((uint8_t*)new_block + sizeof(BlockHeader));
}

void Heap::free(void* ptr) {
    if (!ptr) return;

    BlockHeader* block = (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));
    block->free = true;
    heap_used -= block->size;

    BlockHeader* curr = heap_start->next;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += sizeof(BlockHeader) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

uint32_t Heap::get_used() { return heap_used; }
uint32_t Heap::get_free() { return Heap::HEAP_SIZE - heap_used; }

}

void* operator new(uint32_t size) {
    return Memory::Heap::allocate(size);
}

void operator delete(void* ptr) {
    Memory::Heap::free(ptr);
}

void operator delete(void* ptr, uint32_t) {
    Memory::Heap::free(ptr);
}

void* operator new[](uint32_t size) {
    return Memory::Heap::allocate(size);
}

void operator delete[](void* ptr) {
    Memory::Heap::free(ptr);
}

void operator delete[](void* ptr, uint32_t) {
    Memory::Heap::free(ptr);
}