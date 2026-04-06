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

// tcp_checksum: standard complex checksum with pseudo-header fr
uint16_t TCP::tcp_checksum(const uint8_t* src_ip, const uint8_t* dst_ip, const void* tcp_seg, uint16_t tcp_len) {
    uint32_t sum = 0;

    // pseudo-header bits idc about overhead fr
    const uint16_t* s = (const uint16_t*)src_ip;
    const uint16_t* d = (const uint16_t*)dst_ip;
    sum += s[0]; sum += s[1];
    sum += d[0]; sum += d[1];
    sum += htons(6); // tcp protocol num
    sum += htons(tcp_len);

    // tcp segment data
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

// send_packet: build tcp header and push to ipv4; ts is basic fr
void TCP::send_packet(TCPConnection* conn, uint8_t flags, const void* data, uint16_t len) {
    uint8_t segment[1480];
    TCPHeader* hdr = (TCPHeader*)segment;

    hdr->src_port = htons(conn->local_port);
    hdr->dst_port = htons(conn->remote_port);
    hdr->seq_num = htonl(conn->seq);
    hdr->ack_num = htonl(conn->ack);
    hdr->data_offset = 0x50; // 5 words = 20 bytes fr
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

// handle: main tcp state machine; ts is messy fr
void TCP::handle(const void* data, uint16_t len, const uint8_t* src_ip) {
    if (len < 20) return;
    const TCPHeader* hdr = (const TCPHeader*)data;

    uint16_t src_port = ntohs(hdr->src_port);
    uint16_t dst_port = ntohs(hdr->dst_port);

    // find matching connection idc if its slow
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
                // handshake step 3: send ack fr
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
                // buffer incoming data ngl
                if (conn->rx_len + payload_len <= TCP_RX_BUF_SIZE) {
                    net_memcpy(conn->rx_buf + conn->rx_len, payload, payload_len);
                    conn->rx_len += payload_len;
                    conn->data_ready = true;
                }
                conn->ack = their_seq + payload_len;
                send_packet(conn, TCP_ACK, NULL, 0);
            }
            if ((flags & TCP_ACK) && payload_len == 0) {
                // pure ack; update our seq tracking idc
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

// connect: start 3-way handshake fr
int TCP::connect(const uint8_t* ip, uint16_t port) {
    // find free slot idc
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
    conn->seq = 1000 + slot * 1000; // simple isn ngl
    conn->ack = 0;
    conn->rx_len = 0;
    conn->data_ready = false;
    conn->fin_received = false;

    // send syn fr
    send_packet(conn, TCP_SYN, NULL, 0);
    conn->seq++; // syn consumes seq num idc

    // poll for syn-ack; wait ~500ms fr
    for (int t = 0; t < 500; t++) {
        for (int i = 0; i < 50000; i++) Ethernet::poll();
        if (conn->state == TCP_ESTABLISHED) return slot;
    }

    // timeout; ts is bad news fr
    conn->active = false;
    conn->state = TCP_CLOSED;
    return -1;
}

// send: push data through established connection fr
int TCP::send(int conn_id, const void* data, uint16_t len) {
    if (conn_id < 0 || conn_id >= 4) return -1;
    TCPConnection* conn = &conns[conn_id];
    if (!conn->active || conn->state != TCP_ESTABLISHED) return -1;

    // send in chunks; ts is required for mtu idc
    uint16_t sent = 0;
    while (sent < len) {
        uint16_t chunk = len - sent;
        if (chunk > 1400) chunk = 1400;
        send_packet(conn, TCP_PSH | TCP_ACK, (const uint8_t*)data + sent, chunk);
        conn->seq += chunk;
        sent += chunk;

        // wait briefly for ack idc if it slows down
        for (int i = 0; i < 100000; i++) Ethernet::poll();
    }
    return sent;
}

// receive: pull data from rx buffer fr
int TCP::receive(int conn_id, void* buf, uint16_t max_len) {
    if (conn_id < 0 || conn_id >= 4) return -1;
    TCPConnection* conn = &conns[conn_id];
    if (!conn->active) return -1;

    // poll for data with timeout fr
    for (int t = 0; t < 1000; t++) {
        for (int i = 0; i < 50000; i++) Ethernet::poll();

        if (conn->rx_len > 0) {
            uint16_t copy = conn->rx_len;
            if (copy > max_len) copy = max_len;
            net_memcpy(buf, conn->rx_buf, copy);

            // shift remaining data in buffer idc about efficiency
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
    return 0; // timeout fr
}

// close: start termination handshake fr
void TCP::close(int conn_id) {
    if (conn_id < 0 || conn_id >= 4) return;
    TCPConnection* conn = &conns[conn_id];
    if (!conn->active) return;

    if (conn->state == TCP_ESTABLISHED) {
        send_packet(conn, TCP_FIN | TCP_ACK, NULL, 0);
        conn->seq++;
        conn->state = TCP_FIN_WAIT;

        // wait for fin-ack idc if it takes time
        for (int t = 0; t < 200; t++) {
            for (int i = 0; i < 50000; i++) Ethernet::poll();
            if (conn->state == TCP_CLOSED) break;
        }
    }
    conn->active = false;
    conn->state = TCP_CLOSED;
}
