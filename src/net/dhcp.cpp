#include "net/dhcp.hpp"
#include "net/udp.hpp"
#include "net/ethernet.hpp"
#include "net/ipv4.hpp"
#include "net/netutils.hpp"
#include "drivers/net/rtl8139.hpp"
#include "drivers/video/vga.hpp"

volatile bool DHCP::offer_received = false;
volatile bool DHCP::ack_received = false;
uint32_t DHCP::xid = 0x12345678;
uint8_t DHCP::offered_ip[4] = {0};
uint8_t DHCP::server_ip[4] = {0};

// dhcp packet struct; ts matches rfc 2131 loosely idc
struct DHCPPacket {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint8_t ciaddr[4];
    uint8_t yiaddr[4];
    uint8_t siaddr[4];
    uint8_t giaddr[4];
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
    uint8_t options[312];
} __attribute__((packed));

// build_base_dhcp: setup boilerplate dhcp fields fr
static void build_base_dhcp(DHCPPacket* pkt, uint32_t xid) {
    net_memset(pkt, 0, sizeof(DHCPPacket));
    pkt->op = 1;       // bootrequest opcode fr
    pkt->htype = 1;    // ethernet type
    pkt->hlen = 6;
    pkt->xid = htonl(xid);
    pkt->flags = htons(0x8000); // broadcast bit; ts is required for some servers
    RTL8139::get_mac(pkt->chaddr);

    // dhcp magic cookie fr
    pkt->options[0] = 99;
    pkt->options[1] = 130;
    pkt->options[2] = 83;
    pkt->options[3] = 99;
}

// handle_response: parse incoming dhcp packets; offer/ack idc about NAKs
void DHCP::handle_response(const uint8_t* data, uint16_t len, const uint8_t* /*src_ip*/, uint16_t /*src_port*/) {
    if (len < sizeof(DHCPPacket) - 312) return;
    const DHCPPacket* pkt = (const DHCPPacket*)data;

    if (pkt->op != 2) return; // drop if not reply idc
    if (ntohl(pkt->xid) != xid) return;

    const uint8_t* opts = pkt->options + 4;
    uint8_t msg_type = 0;
    uint8_t subnet[4] = {255,255,255,0};
    uint8_t router[4] = {0};
    uint8_t dns[4] = {0};
    uint8_t srv_id[4] = {0};

    int i = 0;
    while (i < 308) {
        uint8_t opt = opts[i];
        if (opt == 255) break;
        if (opt == 0) { i++; continue; }
        uint8_t opt_len = opts[i + 1];

        if (opt == 53 && opt_len >= 1) msg_type = opts[i + 2];
        if (opt == 1 && opt_len >= 4) net_memcpy(subnet, &opts[i + 2], 4);
        if (opt == 3 && opt_len >= 4) net_memcpy(router, &opts[i + 2], 4);
        if (opt == 6 && opt_len >= 4) net_memcpy(dns, &opts[i + 2], 4);
        if (opt == 54 && opt_len >= 4) net_memcpy(srv_id, &opts[i + 2], 4);

        i += 2 + opt_len;
    }

    if (msg_type == 2) { // offer received fr
        net_memcpy(offered_ip, pkt->yiaddr, 4);
        net_memcpy(server_ip, srv_id, 4);
        offer_received = true;
    } else if (msg_type == 5) { // ack received; we are good ngl
        net_memcpy(net_config.ip, pkt->yiaddr, 4);
        net_memcpy(net_config.subnet, subnet, 4);
        net_memcpy(net_config.gateway, router, 4);
        net_memcpy(net_config.dns, dns, 4);
        net_config.configured = true;
        ack_received = true;
    }
}

// discover: run dhcp state machine (discover -> request) fr
bool DHCP::discover() {
    offer_received = false;
    ack_received = false;

    // bind client port idc if it's already bound
    UDP::bind(68, handle_response);

    // stage 1: dhcp discover fr
    DHCPPacket discover;
    build_base_dhcp(&discover, xid);
    int opt_i = 4;
    discover.options[opt_i++] = 53; discover.options[opt_i++] = 1; discover.options[opt_i++] = 1;
    discover.options[opt_i++] = 55; discover.options[opt_i++] = 3;
    discover.options[opt_i++] = 1; discover.options[opt_i++] = 3; discover.options[opt_i++] = 6;
    discover.options[opt_i++] = 255;

    VGA::println("  Sending DHCP Discover...");
    uint8_t bcast_ip[4] = {255,255,255,255};
    UDP::send(bcast_ip, 68, 67, &discover, sizeof(DHCPPacket));

    // wait for offer; ~500ms fr
    for (int t = 0; t < 500; t++) {
        for (int i = 0; i < 50000; i++) Ethernet::poll();
        if (offer_received) break;
    }
    if (!offer_received) return false;

    VGA::println("  DHCP Offer received!");

    // stage 2: dhcp request fr
    DHCPPacket request;
    build_base_dhcp(&request, xid);
    opt_i = 4;
    request.options[opt_i++] = 53; request.options[opt_i++] = 1; request.options[opt_i++] = 3;
    request.options[opt_i++] = 50; request.options[opt_i++] = 4;
    net_memcpy(&request.options[opt_i], offered_ip, 4); opt_i += 4;
    request.options[opt_i++] = 54; request.options[opt_i++] = 4;
    net_memcpy(&request.options[opt_i], server_ip, 4); opt_i += 4;
    request.options[opt_i++] = 255;

    VGA::println("  Sending DHCP Request...");
    UDP::send(bcast_ip, 68, 67, &request, sizeof(DHCPPacket));

    // wait for ack; another ~500ms fr
    for (int t = 0; t < 500; t++) {
        for (int i = 0; i < 50000; i++) Ethernet::poll();
        if (ack_received) break;
    }

    return ack_received;
}
