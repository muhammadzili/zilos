#pragma once
#include "utils/types.hpp"
#include "drivers/pci.hpp"

#define RTL8139_VENDOR 0x10EC
#define RTL8139_DEVICE 0x8139

#define RX_BUF_SIZE 8192 + 16 + 1500

class RTL8139 {
public:
    static bool init();
    static bool send_packet(const void* data, uint16_t len);
    static int receive_packet(void* buffer, uint16_t max_len);
    static void get_mac(uint8_t* mac_out);
    static bool is_available();

private:
    static PCIDevice dev;
    static uint16_t io_base;
    static uint8_t mac_addr[6];
    static uint8_t rx_buffer[RX_BUF_SIZE];
    static uint32_t rx_offset;
    static uint8_t tx_cur;
    static bool available;
};
