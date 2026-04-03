#pragma once
#include "utils/types.hpp"

struct UDPHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint16_t length;
    uint16_t checksum;
} __attribute__((packed));

// Simple callback system for UDP listeners
#define MAX_UDP_LISTENERS 4
typedef void (*udp_callback_t)(const uint8_t* data, uint16_t len, const uint8_t* src_ip, uint16_t src_port);

class UDP {
public:
    static void send(const uint8_t* dst_ip, uint16_t src_port, uint16_t dst_port, const void* data, uint16_t len);
    static void handle(const void* data, uint16_t len, const uint8_t* src_ip);
    static void bind(uint16_t port, udp_callback_t callback);

private:
    static uint16_t listen_ports[MAX_UDP_LISTENERS];
    static udp_callback_t callbacks[MAX_UDP_LISTENERS];
};
