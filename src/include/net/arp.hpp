#pragma once
#include "utils/types.hpp"

#define ARP_CACHE_SIZE 16

struct ARPEntry {
    uint8_t ip[4];
    uint8_t mac[6];
    bool valid;
};

class ARP {
public:
    static void init();
    static void handle(const void* data, uint16_t len, const uint8_t* sender_mac);
    static bool resolve(const uint8_t* ip, uint8_t* mac_out);
    static void request(const uint8_t* target_ip);

private:
    static ARPEntry cache[ARP_CACHE_SIZE];
    static void cache_add(const uint8_t* ip, const uint8_t* mac);
};
