#include "net/ipv4.hpp"
#include "net/ethernet.hpp"
#include "net/arp.hpp"
#include "net/netutils.hpp"
#include "net/udp.hpp"
#include "net/tcp.hpp"

uint16_t IPv4::packet_id = 1;
volatile bool IPv4::icmp_received = false;

uint16_t IPv4::checksum(const void* data, uint16_t len) {
    uint32_t sum = 0;
    const uint16_t* ptr = (const uint16_t*)data;
    while (len > 1) {
        sum += *ptr++;
        len -= 2;
    }
    if (len == 1) {
        sum += *(const uint8_t*)ptr;
    }
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)(~sum);
}

void IPv4::send(const uint8_t* dst_ip, uint8_t protocol, const void* payload, uint16_t len) {
    uint8_t packet[1500];
    IPv4Header* hdr = (IPv4Header*)packet;

    hdr->ihl_version = 0x45; // Version 4, IHL 5 (20 bytes)
    hdr->tos = 0;
    hdr->total_length = htons(20 + len);
    hdr->id = htons(packet_id++);
    hdr->flags_fragment = htons(0x4000); // Don't Fragment
    hdr->ttl = 64;
    hdr->protocol = protocol;
    hdr->checksum = 0;
    net_memcpy(hdr->src_ip, net_config.ip, 4);
    net_memcpy(hdr->dst_ip, dst_ip, 4);
    hdr->checksum = checksum(hdr, 20);

    if (len > 1480) len = 1480;
    net_memcpy(packet + 20, payload, len);

    // Determine next-hop MAC via ARP
    uint8_t next_hop[4];
    uint32_t dst = ip_to_u32(dst_ip);
    
    // Broadcast?
    if (dst == 0xFFFFFFFF) {
        net_memset(next_hop, 0xFF, 4);
    } else if (net_config.configured) {
        uint32_t src = ip_to_u32(net_config.ip);
        uint32_t mask = ip_to_u32(net_config.subnet);
        if ((dst & mask) == (src & mask)) {
            net_memcpy(next_hop, dst_ip, 4);
        } else {
            net_memcpy(next_hop, net_config.gateway, 4);
        }
    } else {
        // Not configured yet (e.g. DHCP), use destination as next hop
        net_memcpy(next_hop, dst_ip, 4);
    }

    uint8_t dst_mac[6];
    if (ARP::resolve(next_hop, dst_mac)) {
        Ethernet::send(dst_mac, ETH_TYPE_IPV4, packet, 20 + len);
    }
}

void IPv4::handle(const void* data, uint16_t len) {
    if (len < 20) return;
    const IPv4Header* hdr = (const IPv4Header*)data;

    // Only handle IPv4
    if ((hdr->ihl_version >> 4) != 4) return;

    uint8_t header_len = (hdr->ihl_version & 0x0F) * 4;
    const uint8_t* payload = (const uint8_t*)data + header_len;
    uint16_t payload_len = ntohs(hdr->total_length) - header_len;

    if (hdr->protocol == IP_PROTO_ICMP) {
        // Handle ICMP (ping)
        if (payload_len >= 8) {
            uint8_t type = payload[0];
            if (type == 8) { // Echo Request → send Echo Reply
                uint8_t reply[1500];
                net_memcpy(reply, payload, payload_len);
                reply[0] = 0; // Echo Reply type
                reply[2] = 0; reply[3] = 0; // Clear checksum
                uint16_t cksum = checksum(reply, payload_len);
                reply[2] = cksum & 0xFF;
                reply[3] = (cksum >> 8) & 0xFF;
                send(hdr->src_ip, IP_PROTO_ICMP, reply, payload_len);
            } else if (type == 0) { // Echo Reply
                icmp_received = true;
            }
        }
    } else if (hdr->protocol == IP_PROTO_UDP) {
        UDP::handle(payload, payload_len, hdr->src_ip);
    } else if (hdr->protocol == IP_PROTO_TCP) {
        TCP::handle(payload, payload_len, hdr->src_ip);
    }
}
