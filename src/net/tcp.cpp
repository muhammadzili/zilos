#include "net/tcp.hpp"
#include "net/ipv4.hpp"
#include "net/ethernet.hpp"
#include "net/netutils.hpp"

TCPConnection TCP::conns[4];
uint16_t TCP::next_port = 49152;

void TCP::init() {
    for (int i = 0; i < 4; i++) {
        conns[i].active = false;
        conns[i].state = TCP_CLOSED;
        conns[i].rx_len = 0;
        conns[i].data_ready = false;
        conns[i].fin_received = false;
    }
}

uint16_t TCP::tcp_checksum(const uint8_t* src_ip, const uint8_t* dst_ip, const void* tcp_seg, uint16_t tcp_len) {
    uint32_t sum = 0;

    // Pseudo-header
    const uint16_t* s = (const uint16_t*)src_ip;
    const uint16_t* d = (const uint16_t*)dst_ip;
    sum += s[0]; sum += s[1];
    sum += d[0]; sum += d[1];
    sum += htons(6); // TCP protocol
    sum += htons(tcp_len);

    // TCP segment
    const uint16_t* p = (const uint16_t*)tcp_seg;
    uint16_t rem = tcp_len;
    while (rem > 1) {
        sum += *p++;
        rem -= 2;
    }
    if (rem == 1) sum += *(const uint8_t*)p;

    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)(~sum);
}

void TCP::send_packet(TCPConnection* conn, uint8_t flags, const void* data, uint16_t len) {
    uint8_t segment[1480];
    TCPHeader* hdr = (TCPHeader*)segment;

    hdr->src_port = htons(conn->local_port);
    hdr->dst_port = htons(conn->remote_port);
    hdr->seq_num = htonl(conn->seq);
    hdr->ack_num = htonl(conn->ack);
    hdr->data_offset = 0x50; // 5 words = 20 bytes, no options
    hdr->flags = flags;
    hdr->window = htons(TCP_RX_BUF_SIZE);
    hdr->checksum = 0;
    hdr->urgent = 0;

    if (len > 0 && data) {
        net_memcpy(segment + 20, data, len);
    }

    hdr->checksum = tcp_checksum(net_config.ip, conn->remote_ip, segment, 20 + len);

    IPv4::send(conn->remote_ip, IP_PROTO_TCP, segment, 20 + len);
}

void TCP::handle(const void* data, uint16_t len, const uint8_t* src_ip) {
    if (len < 20) return;
    const TCPHeader* hdr = (const TCPHeader*)data;

    uint16_t src_port = ntohs(hdr->src_port);
    uint16_t dst_port = ntohs(hdr->dst_port);

    // Find matching connection
    TCPConnection* conn = NULL;
    for (int i = 0; i < 4; i++) {
        if (conns[i].active &&
            conns[i].local_port == dst_port &&
            conns[i].remote_port == src_port &&
            net_memcmp(conns[i].remote_ip, src_ip, 4)) {
            conn = &conns[i];
            break;
        }
    }
    if (!conn) return;

    uint8_t flags = hdr->flags;
    uint32_t their_seq = ntohl(hdr->seq_num);
    uint32_t their_ack = ntohl(hdr->ack_num);
    uint8_t header_len = (hdr->data_offset >> 4) * 4;
    const uint8_t* payload = (const uint8_t*)data + header_len;
    uint16_t payload_len = len - header_len;

    switch (conn->state) {
        case TCP_SYN_SENT:
            if ((flags & TCP_SYN) && (flags & TCP_ACK)) {
                conn->ack = their_seq + 1;
                conn->seq = their_ack;
                conn->state = TCP_ESTABLISHED;
                // Send ACK
                send_packet(conn, TCP_ACK, NULL, 0);
            }
            break;

        case TCP_ESTABLISHED:
            if (flags & TCP_FIN) {
                conn->ack = their_seq + 1;
                conn->fin_received = true;
                send_packet(conn, TCP_ACK, NULL, 0);
                conn->state = TCP_CLOSE_WAIT;
            }
            if (payload_len > 0) {
                // Buffer incoming data
                if (conn->rx_len + payload_len <= TCP_RX_BUF_SIZE) {
                    net_memcpy(conn->rx_buf + conn->rx_len, payload, payload_len);
                    conn->rx_len += payload_len;
                    conn->data_ready = true;
                }
                conn->ack = their_seq + payload_len;
                send_packet(conn, TCP_ACK, NULL, 0);
            }
            if ((flags & TCP_ACK) && payload_len == 0) {
                // Pure ACK, update our seq tracking
                conn->seq = their_ack;
            }
            break;

        case TCP_FIN_WAIT:
            if (flags & TCP_ACK) {
                if (flags & TCP_FIN) {
                    conn->ack = their_seq + 1;
                    send_packet(conn, TCP_ACK, NULL, 0);
                    conn->state = TCP_CLOSED;
                    conn->active = false;
                }
            }
            break;

        default:
            break;
    }
}

int TCP::connect(const uint8_t* ip, uint16_t port) {
    // Find free slot
    int slot = -1;
    for (int i = 0; i < 4; i++) {
        if (!conns[i].active) { slot = i; break; }
    }
    if (slot < 0) return -1;

    TCPConnection* conn = &conns[slot];
    conn->active = true;
    conn->state = TCP_SYN_SENT;
    net_memcpy(conn->remote_ip, ip, 4);
    conn->local_port = next_port++;
    conn->remote_port = port;
    conn->seq = 1000 + slot * 1000; // Simple ISN
    conn->ack = 0;
    conn->rx_len = 0;
    conn->data_ready = false;
    conn->fin_received = false;

    // Send SYN
    send_packet(conn, TCP_SYN, NULL, 0);
    conn->seq++; // SYN consumes a sequence number

    // Wait for SYN-ACK
    for (int t = 0; t < 500; t++) {
        for (int i = 0; i < 50000; i++) Ethernet::poll();
        if (conn->state == TCP_ESTABLISHED) return slot;
    }

    // Timeout
    conn->active = false;
    conn->state = TCP_CLOSED;
    return -1;
}

int TCP::send(int conn_id, const void* data, uint16_t len) {
    if (conn_id < 0 || conn_id >= 4) return -1;
    TCPConnection* conn = &conns[conn_id];
    if (!conn->active || conn->state != TCP_ESTABLISHED) return -1;

    // Send in chunks
    uint16_t sent = 0;
    while (sent < len) {
        uint16_t chunk = len - sent;
        if (chunk > 1400) chunk = 1400;
        send_packet(conn, TCP_PSH | TCP_ACK, (const uint8_t*)data + sent, chunk);
        conn->seq += chunk;
        sent += chunk;

        // Wait briefly for ACK
        for (int i = 0; i < 100000; i++) Ethernet::poll();
    }
    return sent;
}

int TCP::receive(int conn_id, void* buf, uint16_t max_len) {
    if (conn_id < 0 || conn_id >= 4) return -1;
    TCPConnection* conn = &conns[conn_id];
    if (!conn->active) return -1;

    // Poll for data
    for (int t = 0; t < 1000; t++) {
        for (int i = 0; i < 50000; i++) Ethernet::poll();

        if (conn->rx_len > 0) {
            uint16_t copy = conn->rx_len;
            if (copy > max_len) copy = max_len;
            net_memcpy(buf, conn->rx_buf, copy);

            // Shift remaining data
            uint16_t remaining = conn->rx_len - copy;
            if (remaining > 0) {
                for (uint16_t i = 0; i < remaining; i++)
                    conn->rx_buf[i] = conn->rx_buf[copy + i];
            }
            conn->rx_len = remaining;
            conn->data_ready = remaining > 0;
            return copy;
        }

        if (conn->fin_received || conn->state == TCP_CLOSED) return 0;
    }
    return 0; // Timeout
}

void TCP::close(int conn_id) {
    if (conn_id < 0 || conn_id >= 4) return;
    TCPConnection* conn = &conns[conn_id];
    if (!conn->active) return;

    if (conn->state == TCP_ESTABLISHED) {
        send_packet(conn, TCP_FIN | TCP_ACK, NULL, 0);
        conn->seq++;
        conn->state = TCP_FIN_WAIT;

        // Wait for FIN-ACK
        for (int t = 0; t < 200; t++) {
            for (int i = 0; i < 50000; i++) Ethernet::poll();
            if (conn->state == TCP_CLOSED) break;
        }
    }
    conn->active = false;
    conn->state = TCP_CLOSED;
}
