#include "net/http.hpp"
#include "net/tcp.hpp"
#include "net/dns.hpp"
#include "net/netutils.hpp"
#include "fs/vfs.hpp"
#include "drivers/vga.hpp"

int HTTP::str_len(const char* s) {
    int l = 0;
    while (s[l]) l++;
    return l;
}

void HTTP::str_cpy(char* dst, const char* src) {
    while (*src) *dst++ = *src++;
    *dst = '\0';
}

bool HTTP::parse_url(const char* url, char* host, char* path, uint16_t* port) {
    *port = 80;
    // Skip "http://"
    int i = 0;
    if (url[0]=='h' && url[1]=='t' && url[2]=='t' && url[3]=='p' &&
        url[4]==':' && url[5]=='/' && url[6]=='/') {
        i = 7;
    }

    // Extract host
    int h = 0;
    while (url[i] && url[i] != '/' && url[i] != ':') {
        host[h++] = url[i++];
    }
    host[h] = '\0';

    // Optional port
    if (url[i] == ':') {
        i++;
        *port = 0;
        while (url[i] >= '0' && url[i] <= '9') {
            *port = *port * 10 + (url[i] - '0');
            i++;
        }
    }

    // Path
    if (url[i] == '/') {
        str_cpy(path, url + i);
    } else {
        path[0] = '/';
        path[1] = '\0';
    }
    return h > 0;
}

bool HTTP::get(const char* url, const char* save_filename) {
    char host[128] = {0};
    char path[256] = {0};
    uint16_t port = 80;

    if (!parse_url(url, host, path, &port)) {
        VGA::println("Error: Invalid URL");
        return false;
    }

    // Resolve hostname to IP
    uint8_t server_ip[4];
    if (ip_parse(host, server_ip)) {
        // host is already an IP
    } else {
        VGA::print("Resolving "); VGA::print(host); VGA::println("...");
        if (!DNS::resolve(host, server_ip)) {
            VGA::println("Error: DNS resolution failed");
            return false;
        }
    }

    char ip_str[16];
    ip_to_str(server_ip, ip_str);
    VGA::print("Connecting to "); VGA::print(ip_str); VGA::println("...");

    // TCP connect
    int conn = TCP::connect(server_ip, port);
    if (conn < 0) {
        VGA::println("Error: TCP connection failed");
        return false;
    }

    VGA::println("Connected. Sending HTTP GET...");

    // Build HTTP GET request
    char request[512];
    int pos = 0;

    const char* get_str = "GET ";
    for (int i = 0; get_str[i]; i++) request[pos++] = get_str[i];
    for (int i = 0; path[i]; i++) request[pos++] = path[i];

    const char* http_ver = " HTTP/1.0\r\nHost: ";
    for (int i = 0; http_ver[i]; i++) request[pos++] = http_ver[i];
    for (int i = 0; host[i]; i++) request[pos++] = host[i];

    const char* conn_close = "\r\nConnection: close\r\n\r\n";
    for (int i = 0; conn_close[i]; i++) request[pos++] = conn_close[i];
    request[pos] = '\0';

    TCP::send(conn, request, pos);

    // Receive response
    char response[MAX_FILESIZE];
    int total = 0;
    int received;

    while ((received = TCP::receive(conn, response + total, MAX_FILESIZE - total - 1)) > 0) {
        total += received;
        if (total >= MAX_FILESIZE - 1) break;
    }
    response[total] = '\0';

    TCP::close(conn);

    if (total == 0) {
        VGA::println("Error: No data received");
        return false;
    }

    // Find body (after \r\n\r\n)
    char* body = response;
    for (int i = 0; i < total - 3; i++) {
        if (response[i] == '\r' && response[i+1] == '\n' &&
            response[i+2] == '\r' && response[i+3] == '\n') {
            body = response + i + 4;
            break;
        }
    }

    // Save body to VFS
    VFile* existing = VFS::get_file(save_filename);
    if (!existing) {
        VFS::create_file(save_filename, false);
    }
    VFS::write_file(save_filename, body);

    int body_len = total - (body - response);
    char buf[16];
    // Simple itoa for body_len
    int bi = 0;
    int tmp = body_len;
    if (tmp == 0) buf[bi++] = '0';
    else {
        char rev[16]; int ri = 0;
        while (tmp > 0) { rev[ri++] = '0' + (tmp % 10); tmp /= 10; }
        for (int i = ri - 1; i >= 0; i--) buf[bi++] = rev[i];
    }
    buf[bi] = '\0';

    VGA::print("Downloaded "); VGA::print(buf); VGA::print(" bytes → ");
    VGA::println(save_filename);

    return true;
}
