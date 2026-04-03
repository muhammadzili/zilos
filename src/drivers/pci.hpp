#pragma once
#include "utils/types.hpp"

struct PCIDevice {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint32_t bar0;
    uint8_t irq;
};

class PCI {
public:
    static uint32_t read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
    static void write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
    static bool find_device(uint16_t vendor_id, uint16_t device_id, PCIDevice* out);
    static void enable_bus_mastering(PCIDevice* dev);
};
