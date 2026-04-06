#include "net/arp.hpp"
#include "net/ethernet.hpp"
#include "net/netutils.hpp"
#include "drivers/net/rtl8139.hpp"

// arp packet structure based on rfc 826 idc about formality fr
struct ARPPacket {
    uint16_t hw_type;
    uint16_t proto_type;
    uint8_t hw_size;
    uint8_t proto_size;
    uint16_t opcode;
    uint8_t sender_mac[6];
    uint8_t sender_ip[4];
    uint8_t target_mac[6];
    uint8_t target_ip[4];
} __attribute__((packed));

ARPEntry ARP::cache[ARP_CACHE_SIZE];

void ARP::init() {
    for (int i = 0; i < ARP_CACHE_SIZE; i++) cache[i].valid = false;
}

// cache_add: store ip-mac mapping; overwrite oldest if full idc
void ARP::cache_add(const uint8_t* ip, const uint8_t* mac) {
    // check if already cached and update if so
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (cache[i].valid && net_memcmp(cache[i].ip, ip, 4)) {
            net_memcpy(cache[i].mac, mac, 6);
            return;
        }
    }
    // find empty slot fr
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (!cache[i].valid) {
            cache[i].valid = true;
            net_memcpy(cache[i].ip, ip, 4);
            net_memcpy(cache[i].mac, mac, 6);
            return;
        }
    }
    // cache full; nuke the first one ig
    cache[0].valid = true;
    net_memcpy(cache[0].ip, ip, 4);
    net_memcpy(cache[0].mac, mac, 6);
}

// handle: process incoming arp packets; reply if its for us
void ARP::handle(const void* data, uint16_t len, const uint8_t* /*sender_eth*/) {
    if (len < sizeof(ARPPacket)) return;
    const ARPPacket* arp = (const ARPPacket*)data;

    uint16_t opcode = ntohs(arp->opcode);

    // always cache the sender info ngl
    cache_add(arp->sender_ip, arp->sender_mac);

    if (opcode == 1) { // arp request
        // are they asking for our ip? fr?
        if (net_config.configured && net_memcmp(arp->target_ip, net_config.ip, 4)) {
            // send arp reply fr
            ARPPacket reply;
            reply.hw_type = htons(1);
            reply.proto_type = htons(0x0800);
            reply.hw_size = 6;
            reply.proto_size = 4;
            reply.opcode = htons(2); // reply opcode
            RTL8139::get_mac(reply.sender_mac);
            net_memcpy(reply.sender_ip, net_config.ip, 4);
            net_memcpy(reply.target_mac, arp->sender_mac, 6);
            net_memcpy(reply.target_ip, arp->sender_ip, 4);

            Ethernet::send(arp->sender_mac, ETH_TYPE_ARP, &reply, sizeof(reply));
        }
    }
    // opcode 2 (reply) handled by cache_add earlier idc
}

// resolve: get mac for ip; broadcast request if not in cache fr
bool ARP::resolve(const uint8_t* ip, uint8_t* mac_out) {
    // check if its broadcast
    if (ip[0] == 255 && ip[1] == 255 && ip[2] == 255 && ip[3] == 255) {
        for (int i = 0; i < 6; i++) mac_out[i] = 0xFF;
        return true;
    }

    // hit the cache first
    for (int i = 0; i < ARP_CACHE_SIZE; i++) {
        if (cache[i].valid && net_memcmp(cache[i].ip, ip, 4)) {
            net_memcpy(mac_out, cache[i].mac, 6);
            return true;
        }
    }

    // cache miss; fire request and wait ngl
    request(ip);
    for (int attempt = 0; attempt < 50; attempt++) {
        // poll stack for ~500ms total idc if it lags
        for (int i = 0; i < 10000; i++) {
            Ethernet::poll();
        }
        for (int i = 0; i < ARP_CACHE_SIZE; i++) {
            if (cache[i].valid && net_memcmp(cache[i].ip, ip, 4)) {
                net_memcpy(mac_out, cache[i].mac, 6);
                return true;
            }
        }
    }
    return false; // timeout; ts is bad news
}

// request: send broadcast arp request for ip
void ARP::request(const uint8_t* target_ip) {
    ARPPacket arp;
    arp.hw_type = htons(1);       // ethernet
    arp.proto_type = htons(0x0800); // ipv4
    arp.hw_size = 6;
    arp.proto_size = 4;
    arp.opcode = htons(1);        // request opcode

    RTL8139::get_mac(arp.sender_mac);
    net_memcpy(arp.sender_ip, net_config.ip, 4);
    net_memset(arp.target_mac, 0x00, 6);
    net_memcpy(arp.target_ip, target_ip, 4);

    uint8_t broadcast[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    Ethernet::send(broadcast, ETH_TYPE_ARP, &arp, sizeof(arp));
}
