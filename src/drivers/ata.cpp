#include "drivers/ata.hpp"
#include "utils/types.hpp"
#include "utils/ports.hpp"

bool ATA::drive_present = false;

// Select drive and wait 400ns (read alternate status 4 times)
void ATA::select_drive(uint8_t head_bits) {
    outb(0x1F6, 0xE0 | (head_bits & 0x0F));
    // 400ns delay by reading alternate status register
    inb(0x3F6); inb(0x3F6); inb(0x3F6); inb(0x3F6);
}

bool ATA::wait_bsy() {
    for (int i = 0; i < 1000000; i++) {
        uint8_t status = inb(0x1F7);
        if (!(status & 0x80)) return true;
        // Tiny delay
        for(int d=0; d<100; d++) asm volatile("nop");
    }
    return false;
}

bool ATA::wait_drq() {
    for (int i = 0; i < 1000000; i++) {
        uint8_t status = inb(0x1F7);
        if (status & 0x01) return false; // ERR bit set
        if (status & 0x08) return true;  // DRQ bit set
        // Tiny delay
        for(int d=0; d<100; d++) asm volatile("nop");
    }
    return false;
}

bool ATA::is_present() {
    return drive_present;
}

bool ATA::init() {
    // Try Primary Master first
    if (probe(0)) {
        drive_present = true;
        return true;
    }
    
    // Try Primary Slave
    if (probe(1)) {
        drive_present = true;
        return true;
    }

    drive_present = false;
    return false;
}

bool ATA::probe(uint8_t drive_type) {
    // drive_type: 0 for Master, 1 for Slave
    select_drive(drive_type << 4);
    
    uint8_t status = inb(0x1F7);
    if (status == 0xFF) return false; // Floating bus
    
    // Send IDENTIFY
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    outb(0x1F7, 0xEC); // IDENTIFY
    
    status = inb(0x1F7);
    if (status == 0) return false;
    
    if (!wait_bsy()) return false;
    
    // Check for non-ATA devices
    if (inb(0x1F4) != 0 || inb(0x1F5) != 0) return false;
    
    if (!wait_drq()) return false;
    
    // Read and discard identification data
    for (int i = 0; i < 256; i++) inw(0x1F0);
    
    return true;
}

bool ATA::read_sectors(uint32_t target_address, uint32_t lba, uint32_t sector_count) {
    if (!drive_present) return false;
    
    uint16_t* target = (uint16_t*) target_address;

    for (uint32_t s = 0; s < sector_count; s++) {
        uint32_t cur_lba = lba + s;
        
        select_drive(((cur_lba >> 24) & 0x0F));
        if (!wait_bsy()) return false;
        
        outb(0x1F2, 1);
        outb(0x1F3, (uint8_t)(cur_lba));
        outb(0x1F4, (uint8_t)(cur_lba >> 8));
        outb(0x1F5, (uint8_t)(cur_lba >> 16));
        outb(0x1F7, 0x20); // READ

        // Wait 400ns
        for(int d=0; d<4; d++) inb(0x3F6);

        if (!wait_bsy()) return false;
        if (!wait_drq()) return false;
        
        for (int j = 0; j < 256; j++) {
            target[s * 256 + j] = inw(0x1F0);
        }
    }
    return true;
}

bool ATA::write_sectors(uint32_t target_address, uint32_t lba, uint32_t sector_count) {
    if (!drive_present) return false;
    
    uint16_t* target = (uint16_t*) target_address;

    for (uint32_t s = 0; s < sector_count; s++) {
        uint32_t cur_lba = lba + s;
        
        select_drive(((cur_lba >> 24) & 0x0F));
        if (!wait_bsy()) return false;
        
        outb(0x1F2, 1);
        outb(0x1F3, (uint8_t)(cur_lba));
        outb(0x1F4, (uint8_t)(cur_lba >> 8));
        outb(0x1F5, (uint8_t)(cur_lba >> 16));
        outb(0x1F7, 0x30); // WRITE

        // Wait 400ns
        for(int d=0; d<4; d++) inb(0x3F6);

        if (!wait_bsy()) return false;
        if (!wait_drq()) return false;

        for (int j = 0; j < 256; j++) {
            outw(0x1F0, target[s * 256 + j]);
        }
    }
    
    // Flush cache ONCE at the end
    outb(0x1F7, 0xE7);
    wait_bsy();
    
    return true;
}

