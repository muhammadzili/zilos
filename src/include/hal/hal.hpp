#ifndef HAL_HPP
#define HAL_HPP

#include "hal/block_device.hpp"

class HAL {
public:
    static void set_block_device(BlockDevice* device);
    static BlockDevice* get_block_device();
private:
    static BlockDevice* current_block_device;
};

#endif
