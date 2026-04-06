#include "kernel/paging.hpp"
#include "kernel/memory.hpp"

namespace Paging {

PageDirectory* VM::kernel_dir = NULL;
PageTable* VM::kernel_tables = NULL;

void VM::initialize() {
    kernel_dir = (PageDirectory*)Memory::PMM::allocate_frame();
    kernel_tables = (PageTable*)Memory::PMM::allocate_frame();

    for (uint32_t i = 0; i < PAGE_DIR_ENTRIES; i++) {
        kernel_dir->entries[i] = 0;
    }

    uint32_t phys_tables = (uint32_t)kernel_tables;
    (void)phys_tables;
    
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t phys_frame = (uint32_t)Memory::PMM::allocate_frame();
        if (!phys_frame) break;
        
        kernel_tables[i].entries[0] = phys_frame | 0x03;
        for (uint32_t j = 1; j < PAGE_TABLE_ENTRIES; j++) {
            uint32_t pf = (uint32_t)Memory::PMM::allocate_frame();
            if (!pf) break;
            kernel_tables[i].entries[j] = pf | 0x03;
        }
        
        kernel_dir->entries[i] = (uint32_t)&kernel_tables[i] | 0x03;
    }

    for (uint32_t i = 0; i < 0x400000; i += PAGE_SIZE) {
        kernel_dir->entries[0x300 + (i >> 22)] = i | 0x03;
    }
}

void VM::map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_idx = (virt >> 22) & 0x3FF;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;

    if ((kernel_dir->entries[pd_idx] & 0x01) == 0) {
        PageTable* pt = (PageTable*)Memory::PMM::allocate_frame();
        for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
            pt->entries[i] = 0;
        }
        kernel_dir->entries[pd_idx] = (uint32_t)pt | 0x03;
    }

    PageTable* pt = (PageTable*)(kernel_dir->entries[pd_idx] & ~0xFFF);
    pt->entries[pt_idx] = (phys & ~0xFFF) | flags;
}

void VM::unmap_page(uint32_t virt) {
    uint32_t pd_idx = (virt >> 22) & 0x3FF;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;

    if (kernel_dir->entries[pd_idx] & 0x01) {
        PageTable* pt = (PageTable*)(kernel_dir->entries[pd_idx] & ~0xFFF);
        pt->entries[pt_idx] = 0;
    }
}

uint32_t* VM::get_page(uint32_t virt, bool make) {
    uint32_t pd_idx = (virt >> 22) & 0x3FF;
    uint32_t pt_idx = (virt >> 12) & 0x3FF;

    if (kernel_dir->entries[pd_idx] & 0x01) {
        PageTable* pt = (PageTable*)(kernel_dir->entries[pd_idx] & ~0xFFF);
        return &pt->entries[pt_idx];
    }

    if (make) {
        PageTable* pt = (PageTable*)Memory::PMM::allocate_frame();
        for (uint32_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
            pt->entries[i] = 0;
        }
        kernel_dir->entries[pd_idx] = (uint32_t)pt | 0x03;
        return &pt->entries[pt_idx];
    }

    return NULL;
}

void VM::enable_paging() {
    asm volatile ("mov %0, %%cr3" : : "r"(kernel_dir));
    uint32_t cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" : : "r"(cr0));
}

}

extern "C" void enable_paging() {
    Paging::VM::enable_paging();
}