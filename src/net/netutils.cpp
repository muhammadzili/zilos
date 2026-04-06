#include "net/netutils.hpp"

NetConfig net_config = {{0},{0},{0},{0},{0},false};

// net_memcpy: low level copy idc if it's slow fr
void net_memcpy(void* dst, const void* src, uint32_t len) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    for (uint32_t i = 0; i < len; i++) d[i] = s[i];
}

// net_memset: fill memory with byte val idc fr
void net_memset(void* dst, uint8_t val, uint32_t len) {
    uint8_t* d = (uint8_t*)dst;
    for (uint32_t i = 0; i < len; i++) d[i] = val;
}

// net_memcmp: check if two buffers match fr
bool net_memcmp(const void* a, const void* b, uint32_t len) {
    const uint8_t* aa = (const uint8_t*)a;
    const uint8_t* bb = (const uint8_t*)b;
    for (uint32_t i = 0; i < len; i++) {
        if (aa[i] != bb[i]) return false;
    }
    return true;
}

// byte order helpers: ts is mandatory for network code fr
uint16_t htons(uint16_t v) { return (v >> 8) | (v << 8); }
uint16_t ntohs(uint16_t v) { return (v >> 8) | (v << 8); }
uint32_t htonl(uint32_t v) {
    return ((v >> 24) & 0xFF) | ((v >> 8) & 0xFF00) |
           ((v << 8) & 0xFF0000) | ((v << 24) & 0xFF000000);
}
uint32_t ntohl(uint32_t v) { return htonl(v); }

// ip_to_u32: pack 4 bytes into 32bit int fr
uint32_t ip_to_u32(const uint8_t* ip) {
    return ((uint32_t)ip[0] << 24) | ((uint32_t)ip[1] << 16) |
           ((uint32_t)ip[2] << 8) | ip[3];
}

// u32_to_ip: unpack 32bit int into 4 ip bytes idc fr
void u32_to_ip(uint32_t val, uint8_t* ip) {
    ip[0] = (val >> 24) & 0xFF;
    ip[1] = (val >> 16) & 0xFF;
    ip[2] = (val >> 8) & 0xFF;
    ip[3] = val & 0xFF;
}

// ip_parse: convert string "a.b.c.d" -> uint8[4] fr
bool ip_parse(const char* str, uint8_t* ip_out) {
    uint8_t part = 0;
    uint16_t val = 0;
    int i = 0;
    while (str[i]) {
        if (str[i] == '.') {
            if (part >= 3) return false;
            ip_out[part++] = (uint8_t)val;
            val = 0;
        } else if (str[i] >= '0' && str[i] <= '9') {
            val = val * 10 + (str[i] - '0');
            if (val > 255) return false;
        } else {
            return false;
        }
        i++;
    }
    if (part != 3) return false;
    ip_out[part] = (uint8_t)val;
    return true;
}

// ip_to_str: convert uint8[4] -> string fr
void ip_to_str(const uint8_t* ip, char* str) {
    int pos = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t v = ip[i];
        if (v >= 100) { str[pos++] = '0' + v / 100; v %= 100; str[pos++] = '0' + v / 10; v %= 10; }
        else if (v >= 10) { str[pos++] = '0' + v / 10; v %= 10; }
        str[pos++] = '0' + v;
        if (i < 3) str[pos++] = '.';
    }
    str[pos] = '\0';
}
