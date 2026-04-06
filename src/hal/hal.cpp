#include "hal/hal.hpp"

BlockDevice* HAL::current_block_device = 0;

void HAL::set_block_device(BlockDevice* device) {
    current_block_device = device;
}

BlockDevice* HAL::get_block_device() {
    return current_block_device;
}
