#pragma once
#include "utils/types.hpp"

// Global network configuration (populated by DHCP)
struct NetConfig {
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t gateway[4];
    uint8_t subnet[4];
    uint8_t dns[4];
    bool configured;
};

extern NetConfig net_config;

// Memory copy/set/compare helpers for networking
void net_memcpy(void* dst, const void* src, uint32_t len);
void net_memset(void* dst, uint8_t val, uint32_t len);
bool net_memcmp(const void* a, const void* b, uint32_t len);

// Convert 16/32 bit between host and network byte order (big-endian swap)
uint16_t htons(uint16_t v);
uint16_t ntohs(uint16_t v);
uint32_t htonl(uint32_t v);
uint32_t ntohl(uint32_t v);

// IP address helpers
uint32_t ip_to_u32(const uint8_t* ip);
void u32_to_ip(uint32_t val, uint8_t* ip);
bool ip_parse(const char* str, uint8_t* ip_out);
void ip_to_str(const uint8_t* ip, char* str);
