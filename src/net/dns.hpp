#pragma once
#include "utils/types.hpp"

class DNS {
public:
    static bool resolve(const char* hostname, uint8_t* ip_out);

private:
    static volatile bool response_received;
    static uint8_t resolved_ip[4];
    static uint16_t query_id;
    static void handle_response(const uint8_t* data, uint16_t len, const uint8_t* src_ip, uint16_t src_port);
};
