#pragma once
#include "utils/types.hpp"

class DHCP {
public:
    static bool discover();

private:
    static volatile bool offer_received;
    static volatile bool ack_received;
    static uint32_t xid;
    static uint8_t offered_ip[4];
    static uint8_t server_ip[4];
    static void handle_response(const uint8_t* data, uint16_t len, const uint8_t* src_ip, uint16_t src_port);
};
