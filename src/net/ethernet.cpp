#include "net/ethernet.hpp"
#include "net/netutils.hpp"
#include "net/arp.hpp"
#include "net/ipv4.hpp"
#include "drivers/rtl8139.hpp"

void Ethernet::send(const uint8_t* dst_mac, uint16_t type, const void* payload, uint16_t len) {
    uint8_t frame[1518];
    EthHeader* hdr = (EthHeader*)frame;

    net_memcpy(hdr->dst, dst_mac, 6);
    RTL8139::get_mac(hdr->src);
    hdr->type = htons(type);

    uint16_t total_len = sizeof(EthHeader) + len;
    if (len > 1500) len = 1500;
    net_memcpy(frame + sizeof(EthHeader), payload, len);

    // Padding to minimum Ethernet frame size (60 bytes excluding CRC)
    if (total_len < 60) {
        net_memset(frame + total_len, 0, 60 - total_len);
        total_len = 60;
    }

    RTL8139::send_packet(frame, total_len);
}

void Ethernet::handle_packet(const void* data, uint16_t len) {
    if (len < sizeof(EthHeader)) return;

    const EthHeader* hdr = (const EthHeader*)data;
    uint16_t type = ntohs(hdr->type);
    const uint8_t* payload = (const uint8_t*)data + sizeof(EthHeader);
    uint16_t payload_len = len - sizeof(EthHeader);

    if (type == ETH_TYPE_ARP) {
        ARP::handle(payload, payload_len, hdr->src);
    } else if (type == ETH_TYPE_IPV4) {
        IPv4::handle(payload, payload_len);
    }
}

static uint8_t poll_buf[1600];

void Ethernet::poll() {
    int len = RTL8139::receive_packet(poll_buf, 1600);
    if (len > 0) {
        handle_packet(poll_buf, len);
    }
}
