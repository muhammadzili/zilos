#pragma once
#include "utils/types.hpp"

#define ETH_TYPE_ARP  0x0806
#define ETH_TYPE_IPV4 0x0800

struct EthHeader {
    uint8_t dst[6];
    uint8_t src[6];
    uint16_t type;
} __attribute__((packed));

class Ethernet {
public:
    static void send(const uint8_t* dst_mac, uint16_t type, const void* payload, uint16_t len);
    static void handle_packet(const void* data, uint16_t len);
    static void poll();
};
