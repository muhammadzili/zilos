#include "net/udp.hpp"
#include "net/ipv4.hpp"
#include "net/netutils.hpp"

uint16_t UDP::listen_ports[MAX_UDP_LISTENERS] = {0};
udp_callback_t UDP::callbacks[MAX_UDP_LISTENERS] = {0};

void UDP::bind(uint16_t port, udp_callback_t callback) {
    for (int i = 0; i < MAX_UDP_LISTENERS; i++) {
        if (listen_ports[i] == 0) {
            listen_ports[i] = port;
            callbacks[i] = callback;
            return;
        }
    }
}

void UDP::send(const uint8_t* dst_ip, uint16_t src_port, uint16_t dst_port, const void* data, uint16_t len) {
    uint8_t packet[1480];
    UDPHeader* hdr = (UDPHeader*)packet;

    hdr->src_port = htons(src_port);
    hdr->dst_port = htons(dst_port);
    hdr->length = htons(8 + len);
    hdr->checksum = 0;

    if (len > 1472) len = 1472;
    net_memcpy(packet + 8, data, len);

    // Calculate UDP Checksum (Pseudo-header + UDP segment)
    uint32_t sum = 0;
    const uint16_t* s = (const uint16_t*)net_config.ip;
    const uint16_t* d = (const uint16_t*)dst_ip;
    sum += s[0]; sum += s[1];
    sum += d[0]; sum += d[1];
    sum += htons(IP_PROTO_UDP);
    sum += hdr->length;

    const uint16_t* p = (const uint16_t*)packet;
    uint16_t rem = 8 + len;
    while (rem > 1) { sum += *p++; rem -= 2; }
    if (rem == 1) sum += *(const uint8_t*)p;

    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    hdr->checksum = (uint16_t)(~sum);
    if (hdr->checksum == 0) hdr->checksum = 0xFFFF;

    IPv4::send(dst_ip, IP_PROTO_UDP, packet, 8 + len);
}

void UDP::handle(const void* data, uint16_t len, const uint8_t* src_ip) {
    if (len < 8) return;
    const UDPHeader* hdr = (const UDPHeader*)data;

    uint16_t dst_port = ntohs(hdr->dst_port);
    uint16_t src_port = ntohs(hdr->src_port);
    const uint8_t* payload = (const uint8_t*)data + 8;
    uint16_t payload_len = ntohs(hdr->length) - 8;

    for (int i = 0; i < MAX_UDP_LISTENERS; i++) {
        if (listen_ports[i] == dst_port && callbacks[i]) {
            callbacks[i](payload, payload_len, src_ip, src_port);
            return;
        }
    }
}
