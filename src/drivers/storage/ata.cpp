#include "drivers/storage/ata.hpp"
#include "utils/ports.hpp"
#include "utils/types.hpp"

bool ATA::drive_present = false;

// select drive: wait 400ns by reading status 4 times lol
void ATA::select_drive(uint8_t head_bits) {
    outb(0x1F6, 0xE0 | (head_bits & 0x0F));
    // 400ns delay idc about the exact timing but 4 reads works fr
    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);
}

// wait_bsy: wait for busy bit to clear
bool ATA::wait_bsy() {
    for (int i = 0; i < 1000000; i++) {
        uint8_t status = inb(0x1F7);
        if (!(status & 0x80)) return true;
        // tiny delay ngl
        for(int d=0; d<100; d++) asm volatile("nop");
    }
    return false;
}

// wait_drq: wait for data request bit to set
bool ATA::wait_drq() {
    for (int i = 0; i < 1000000; i++) {
        uint8_t status = inb(0x1F7);
        if (status & 0x01) return false; // err bit set; ts failed
        if (status & 0x08) return true;  // drq bit set; we good
        // tiny delay
        for(int d=0; d<100; d++) asm volatile("nop");
    }
    return false;
}

bool ATA::is_present() {
    return drive_present;
}

// init: check primary master and slave drives
bool ATA::init() {
    // try primary master first
    if (probe(0)) {
        drive_present = true;
        return true;
    }
    
    // try primary slave
    if (probe(1)) {
        drive_present = true;
        return true;
    }

    drive_present = false;
    return false;
}

// probe: send identify command to drive idc if it fails
bool ATA::probe(uint8_t drive_type) {
    // drive_type: 0 for master, 1 for slave
    select_drive(drive_type << 4);
    
    uint8_t status = inb(0x1F7);
    if (status == 0xFF) return false; // floating bus; nothing there fr
    
    // send identify
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    outb(0x1F7, 0xEC); // identify
    
    status = inb(0x1F7);
    if (status == 0) return false;
    
    if (!wait_bsy()) return false;
    
    // check for non-ata devices
    if (inb(0x1F4) != 0 || inb(0x1F5) != 0) return false;
    
    if (!wait_drq()) return false;
    
    // read and discard ident data idk why we need it yet
    for (int i = 0; i < 256; i++) inw(0x1F0);
    
    return true;
}

// read_sectors: grab data from disk lba style
bool ATA::read_sectors(uint32_t target_address, uint32_t lba, uint32_t count) {
    if (!drive_present) return false;
    
    uint16_t* target = (uint16_t*) target_address;

    for (uint32_t s = 0; s < count; s++) {
        uint32_t cur_lba = lba + s;
        
        select_drive(((cur_lba >> 24) & 0x0F));
        if (!wait_bsy()) return false;
        
        outb(0x1F2, 1);
        outb(0x1F3, (uint8_t)(cur_lba));
        outb(0x1F4, (uint8_t)(cur_lba >> 8));
        outb(0x1F5, (uint8_t)(cur_lba >> 16));
        outb(0x1F7, 0x20); // read cmd

        // wait 400ns
        for(int d=0; d<4; d++) inb(0x3F6);

        if (!wait_bsy()) return false;
        if (!wait_drq()) return false;
        
        for (int j = 0; j < 256; j++) {
            target[s * 256 + j] = inw(0x1F0);
        }
    }
    return true;
}

// write_sectors: push data to disk lba style
bool ATA::write_sectors(uint32_t target_address, uint32_t lba, uint32_t count) {
    if (!drive_present) return false;
    
    uint16_t* target = (uint16_t*) target_address;

    for (uint32_t s = 0; s < count; s++) {
        uint32_t cur_lba = lba + s;
        
        select_drive(((cur_lba >> 24) & 0x0F));
        if (!wait_bsy()) return false;
        
        outb(0x1F2, 1);
        outb(0x1F3, (uint8_t)(cur_lba));
        outb(0x1F4, (uint8_t)(cur_lba >> 8));
        outb(0x1F5, (uint8_t)(cur_lba >> 16));
        outb(0x1F7, 0x30); // write cmd

        // wait 400ns
        for(int d=0; d<4; d++) inb(0x3F6);

        if (!wait_bsy()) return false;
        if (!wait_drq()) return false;

        for (int j = 0; j < 256; j++) {
            outw(0x1F0, target[s * 256 + j]);
        }
    }
    
    // flush cache once at the end idgaf about data loss during debug
    outb(0x1F7, 0xE7);
    wait_bsy();
    
    return true;
}

