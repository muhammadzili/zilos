#pragma once
#include "utils/types.hpp"

struct TCPHeader {
    uint16_t src_port;
    uint16_t dst_port;
    uint32_t seq_num;
    uint32_t ack_num;
    uint8_t  data_offset; // upper 4 bits = offset in 32-bit words
    uint8_t  flags;
    uint16_t window;
    uint16_t checksum;
    uint16_t urgent;
} __attribute__((packed));

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10

enum TCPState {
    TCP_CLOSED,
    TCP_SYN_SENT,
    TCP_ESTABLISHED,
    TCP_FIN_WAIT,
    TCP_CLOSE_WAIT
};

#define TCP_RX_BUF_SIZE 8192

struct TCPConnection {
    bool active;
    TCPState state;
    uint8_t remote_ip[4];
    uint16_t local_port;
    uint16_t remote_port;
    uint32_t seq;       // Our sequence number
    uint32_t ack;       // Their sequence number (what we ack)
    uint8_t rx_buf[TCP_RX_BUF_SIZE];
    uint16_t rx_len;
    bool data_ready;
    bool fin_received;
};

class TCP {
public:
    static void init();
    static void handle(const void* data, uint16_t len, const uint8_t* src_ip);
    static int connect(const uint8_t* ip, uint16_t port);
    static int send(int conn_id, const void* data, uint16_t len);
    static int receive(int conn_id, void* buf, uint16_t max_len);
    static void close(int conn_id);

private:
    static TCPConnection conns[4];
    static uint16_t next_port;
    static void send_packet(TCPConnection* conn, uint8_t flags, const void* data, uint16_t len);
    static uint16_t tcp_checksum(const uint8_t* src_ip, const uint8_t* dst_ip, const void* tcp_seg, uint16_t tcp_len);
};
