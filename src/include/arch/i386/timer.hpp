#pragma once

#include "utils/types.hpp"

namespace Timer {

const uint32_t PIT_CHANNEL_0 = 0x40;
const uint32_t PIT_CMD_PORT = 0x43;
const uint32_t PIT_FREQ = 1193182;

void initialize(uint32_t frequency);
uint32_t get_ticks();
void sleep(uint32_t ms);
void set_handler(void (*handler)(void));

}