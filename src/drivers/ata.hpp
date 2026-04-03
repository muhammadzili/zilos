#pragma once
#include "utils/types.hpp"

class ATA {
public:
    static bool init();
    static bool is_present();
    static bool read_sectors(uint32_t target_address, uint32_t lba, uint32_t sector_count);
    static bool write_sectors(uint32_t target_address, uint32_t lba, uint32_t sector_count);
private:
    static bool probe(uint8_t drive_type);
    static bool wait_bsy();
    static bool wait_drq();
    static void select_drive(uint8_t head_bits);
    static bool drive_present;
};
