#ifndef BLOCK_DEVICE_HPP
#define BLOCK_DEVICE_HPP

#include "utils/types.hpp"

class BlockDevice {
public:
    virtual const char* get_name() = 0;
    virtual bool read_sectors(uint32_t buffer, uint32_t lba, uint32_t count) = 0;
    virtual bool write_sectors(uint32_t buffer, uint32_t lba, uint32_t count) = 0;
    virtual uint32_t get_size_sectors() = 0;
};

#endif
