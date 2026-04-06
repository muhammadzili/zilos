#pragma once

#include "utils/types.hpp"
#include "hal/block_device.hpp"

class ATA : public BlockDevice {
public:
    static bool init();
    
    // blockdevice interface fr
    const char* get_name() override { return "ata-master"; }
    bool read_sectors(uint32_t target_address, uint32_t lba, uint32_t count) override;
    bool write_sectors(uint32_t target_address, uint32_t lba, uint32_t count) override;
    uint32_t get_size_sectors() override { return 131072; } // 64mb idc 
    
    static bool is_present();

private:
    static bool probe(uint8_t drive_type);
    static bool wait_bsy();
    static bool wait_drq();
    static void select_drive(uint8_t head_bits);
    static bool drive_present;
};
