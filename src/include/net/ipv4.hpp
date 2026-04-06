#pragma once
#include "utils/types.hpp"

#define IP_PROTO_ICMP 1
#define IP_PROTO_TCP  6
#define IP_PROTO_UDP  17

struct IPv4Header {
    uint8_t  ihl_version;
    uint8_t  tos;
    uint16_t total_length;
    uint16_t id;
    uint16_t flags_fragment;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint8_t  src_ip[4];
    uint8_t  dst_ip[4];
} __attribute__((packed));

class IPv4 {
public:
    static void send(const uint8_t* dst_ip, uint8_t protocol, const void* payload, uint16_t len);
    static void handle(const void* data, uint16_t len);
    static uint16_t checksum(const void* data, uint16_t len);
    static volatile bool icmp_received;

private:
    static uint16_t packet_id;
};
