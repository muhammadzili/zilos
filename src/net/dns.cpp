#include "net/dns.hpp"
#include "net/udp.hpp"
#include "net/ethernet.hpp"
#include "net/netutils.hpp"

volatile bool DNS::response_received = false;
uint8_t DNS::resolved_ip[4] = {0};
uint16_t DNS::query_id = 0x1234;

void DNS::handle_response(const uint8_t* data, uint16_t len, const uint8_t* /*src_ip*/, uint16_t /*src_port*/) {
    if (len < 12) return;

    uint16_t id = (data[0] << 8) | data[1];
    if (id != query_id) return;

    uint16_t flags = (data[2] << 8) | data[3];
    if (!(flags & 0x8000)) return; // Not a response

    uint16_t qdcount = (data[4] << 8) | data[5];
    uint16_t ancount = (data[6] << 8) | data[7];

    if (ancount == 0) return;

    // Skip header (12 bytes) and question section
    int pos = 12;
    for (uint16_t q = 0; q < qdcount; q++) {
        // Skip QNAME
        while (pos < (int)len && data[pos] != 0) {
            if ((data[pos] & 0xC0) == 0xC0) { pos += 2; goto skip_done; }
            pos += data[pos] + 1;
        }
        pos++; // Skip null terminator
        skip_done:
        pos += 4; // Skip QTYPE + QCLASS
    }

    // Parse answer records
    for (uint16_t a = 0; a < ancount; a++) {
        if (pos >= (int)len) break;

        // Skip NAME (might be a pointer)
        if ((data[pos] & 0xC0) == 0xC0) {
            pos += 2;
        } else {
            while (pos < (int)len && data[pos] != 0) pos += data[pos] + 1;
            pos++;
        }
        if (pos + 10 > (int)len) break;

        uint16_t rtype = (data[pos] << 8) | data[pos + 1];
        uint16_t rdlength = (data[pos + 8] << 8) | data[pos + 9];
        pos += 10;

        if (rtype == 1 && rdlength == 4) { // A record
            net_memcpy(resolved_ip, &data[pos], 4);
            response_received = true;
            return;
        }
        pos += rdlength;
    }
}

bool DNS::resolve(const char* hostname, uint8_t* ip_out) {
    if (!net_config.configured) return false;

    response_received = false;
    query_id++;

    // Bind port 1053 for DNS response
    UDP::bind(1053, handle_response);

    // Build DNS query
    uint8_t query[256];
    int pos = 0;

    // Header
    query[pos++] = (query_id >> 8) & 0xFF;
    query[pos++] = query_id & 0xFF;
    query[pos++] = 0x01; query[pos++] = 0x00; // Flags: standard query, recursion desired
    query[pos++] = 0x00; query[pos++] = 0x01; // QDCOUNT = 1
    query[pos++] = 0x00; query[pos++] = 0x00; // ANCOUNT
    query[pos++] = 0x00; query[pos++] = 0x00; // NSCOUNT
    query[pos++] = 0x00; query[pos++] = 0x00; // ARCOUNT

    // QNAME: convert "example.com" → "\x07example\x03com\x00"
    int label_start = pos;
    pos++; // Reserve space for label length
    int label_len = 0;
    int i = 0;
    while (hostname[i]) {
        if (hostname[i] == '.') {
            query[label_start] = label_len;
            label_start = pos;
            pos++;
            label_len = 0;
        } else {
            query[pos++] = hostname[i];
            label_len++;
        }
        i++;
    }
    query[label_start] = label_len;
    query[pos++] = 0; // End of QNAME

    // QTYPE = A (1), QCLASS = IN (1)
    query[pos++] = 0x00; query[pos++] = 0x01;
    query[pos++] = 0x00; query[pos++] = 0x01;

    // Send query to DNS server on port 53
    UDP::send(net_config.dns, 1053, 53, query, pos);

    // Wait for response
    for (int t = 0; t < 300; t++) {
        for (int j = 0; j < 50000; j++) Ethernet::poll();
        if (response_received) {
            net_memcpy(ip_out, resolved_ip, 4);
            return true;
        }
    }
    return false;
}
