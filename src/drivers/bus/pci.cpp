#include "drivers/bus/pci.hpp"
#include "utils/ports.hpp"

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

// read: grab 32-bit reg from pci config space
uint32_t PCI::read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) |
                       (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDR, address);
    return inl(PCI_CONFIG_DATA);
}

// write: push 32-bit val to pci config space
void PCI::write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) |
                       (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDR, address);
    outl(PCI_CONFIG_DATA, value);
}

// find_device: brute force scan all buses for specific vid/did
bool PCI::find_device(uint16_t vendor_id, uint16_t device_id, PCIDevice* out) {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t reg0 = read(bus, slot, 0, 0);
            uint16_t vid = reg0 & 0xFFFF;
            uint16_t did = (reg0 >> 16) & 0xFFFF;

            if (vid == 0xFFFF) continue; // nothing here idc

            if (vid == vendor_id && did == device_id) {
                out->bus = bus;
                out->slot = slot;
                out->func = 0;
                out->vendor_id = vid;
                out->device_id = did;

                out->bar0 = read(bus, slot, 0, 0x10) & 0xFFFFFFFC; // io base fr
                uint32_t irq_reg = read(bus, slot, 0, 0x3C);
                out->irq = irq_reg & 0xFF;
                return true;
            }
        }
    }
    return false;
}

// enable_bus_mastering: flip the bit so device can do dma stuff
void PCI::enable_bus_mastering(PCIDevice* dev) {
    uint32_t cmd = read(dev->bus, dev->slot, dev->func, 0x04);
    cmd |= (1 << 2); // bme bit; ts is important for nics etc
    write(dev->bus, dev->slot, dev->func, 0x04, cmd);
}
