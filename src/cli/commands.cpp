#include "cli/commands.hpp"
#include "drivers/vga.hpp"
#include "drivers/ata.hpp"
#include "fs/vfs.hpp"
#include "net/dhcp.hpp"
#include "net/netutils.hpp"
#include "apps/editor.hpp"
#include "utils/ports.hpp"
#include "net/netutils.hpp"
#include "net/ethernet.hpp"
#include "net/ipv4.hpp"
#include "net/dns.hpp"
#include "net/http.hpp"
#include "drivers/rtl8139.hpp"
#include "fs/config.hpp"

extern uint32_t system_ram_mb;

bool Commands::strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

void Commands::itoa(size_t value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    size_t tmp_value;
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);
    
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

void Commands::sleep(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 100000; i++) {
        asm volatile("nop");
    }
}

void Commands::execute(const char* cmd, const char* args) {
    if (strcmp(cmd, "help")) {
        cmd_help();
    } else if (strcmp(cmd, "clear")) {
        cmd_clear();
    } else if (strcmp(cmd, "echo")) {
        cmd_echo(args);
    } else if (strcmp(cmd, "about")) {
        cmd_about();
    } else if (strcmp(cmd, "mkdir")) {
        cmd_mkdir(args);
    } else if (strcmp(cmd, "ls")) {
        cmd_ls();
    } else if (strcmp(cmd, "install")) {
        cmd_install();
    } else if (strcmp(cmd, "save")) {
        cmd_save();
    } else if (strcmp(cmd, "cat")) {
        cmd_cat(args);
    } else if (strcmp(cmd, "rm")) {
        cmd_rm(args);
    } else if (strcmp(cmd, "edit")) {
        cmd_edit(args);
    } else if (strcmp(cmd, "cpu")) {
        cmd_cpu();
    } else if (strcmp(cmd, "memory")) {
        cmd_memory();
    } else if (strcmp(cmd, "storage")) {
        cmd_storage();
    } else if (strcmp(cmd, "ifconfig")) {
        cmd_ifconfig();
    } else if (strcmp(cmd, "ping")) {
        cmd_ping(args);
    } else if (strcmp(cmd, "dns")) {
        cmd_dns(args);
    } else if (strcmp(cmd, "wget")) {
        cmd_wget(args);
    } else if (strcmp(cmd, "reboot")) {
        cmd_reboot();
    } else if (strcmp(cmd, "shutdown")) {
        cmd_shutdown();
    } else if (strcmp(cmd, "touch")) {
        cmd_touch(args);
    } else if (strcmp(cmd, "pwd")) {
        cmd_pwd();
    } else if (strcmp(cmd, "whoami")) {
        cmd_whoami();
    } else if (strcmp(cmd, "uname")) {
        cmd_uname();
    } else if (strcmp(cmd, "date")) {
        cmd_date();
    } else if (strcmp(cmd, "df")) {
        cmd_storage();
    } else if (strcmp(cmd, "free")) {
        cmd_memory();
    } else if (strcmp(cmd, "lscpu")) {
        cmd_cpu();
    } else if (strcmp(cmd, "hostname")) {
        cmd_hostname();
    } else if (strcmp(cmd, "uptime")) {
        cmd_uptime();
    } else if (strcmp(cmd, "sleep")) {
        cmd_sleep_cmd(args);
    } else if (strcmp(cmd, "dmesg")) {
        cmd_dmesg();
    } else if (strcmp(cmd, "poweroff") || strcmp(cmd, "halt")) {
        cmd_shutdown();
    } else if (strcmp(cmd, "id")) {
        cmd_whoami();
    } else if (strcmp(cmd, "sync")) {
        cmd_save();
    } else if (strcmp(cmd, "net")) {
        cmd_net(args);
    } else {
        VGA::print("Unknown command: ");
        VGA::println(cmd);
    }
}

void Commands::cmd_help() {
    VGA::println("Available commands:");
    VGA::println("  -- Basic --");
    VGA::println("  help  - Show this help message");
    VGA::println("  clear - Clear the screen");
    VGA::println("  echo  - Print text to the screen");
    VGA::println("  about - Show system information");
    VGA::println("  -- Filesystem & Editor --");
    VGA::println("  ls           - List files in VFS");
    VGA::println("  mkdir <name> - Create an empty folder at VFS");
    VGA::println("  cat <name>   - Read a file from VFS");
    VGA::println("  rm <name>    - Delete a file/folder");
    VGA::println("  touch <name> - Create an empty file");
    VGA::println("  pwd          - Print working directory");
    VGA::println("  install      - Format ATA disk for ZilOS");
    VGA::println("  save         - Save VFS to ATA disk");
    VGA::println("  edit <name>  - Open ZEDIT (Text Editor)");
    VGA::println("  reboot       - Restart ZilOS");
    VGA::println("  shutdown     - Power off ZilOS");
    VGA::println("  -- System --");
    VGA::println("  cpu/lscpu       - Show CPU Vendor info");
    VGA::println("  memory/free     - Show RAM info");
    VGA::println("  storage/df      - Show VFS Storage usage");
    VGA::println("  whoami/id       - Show current user");
    VGA::println("  uname           - Show kernel info");
    VGA::println("  date            - Show system time");
    VGA::println("  hostname        - Show system hostname");
    VGA::println("  uptime          - Show system uptime");
    VGA::println("  dmesg           - Show kernel boot log");
    VGA::println("  sleep <ms>      - Pause execution");
    VGA::println("  -- Network --");
    VGA::println("  net <status|apply> - Manage Network config from /network/config.txt");
    VGA::println("  ifconfig         - Show network configuration");
    VGA::println("  ping <ip>        - Ping an IP address (ICMP)");
    VGA::println("  dns <hostname>   - Resolve hostname to IP");
    VGA::println("  wget <url> <file> - Download file via HTTP");
}

void Commands::cmd_clear() { VGA::clear(); }
void Commands::cmd_echo(const char* args) { VGA::println(args ? args : ""); }
void Commands::cmd_about() {
    VGA::println("ZilOS v4.0 - Network Edition");
    VGA::println("A pure CLI operating system with TCP/IP.");
    VGA::println("Created for Zill.");
}

void Commands::cmd_date() {
    VGA::println("Sun Apr 03 06:00:00 UTC 2026");
    VGA::println("(RTC hook not implemented yet)");
}

void Commands::cmd_pwd() {
    VGA::println("/");
}

void Commands::cmd_uname() {
    VGA::println("ZilOS v4.0-generic i386");
}

void Commands::cmd_whoami() {
    VGA::println(Config::get_username());
}

void Commands::cmd_hostname() {
    VGA::println("zilos-core");
}

void Commands::cmd_uptime() {
    VGA::println("Uptime: Unknown (PIT/RTC driver missing)");
    VGA::println("Load average: 0.00, 0.00, 0.00");
}

void Commands::cmd_dmesg() {
    VGA::println("[0.000000] ZilOS v4.0 CLI Kernel Booting");
    VGA::println("[0.000000] Initializing GDT... OK");
    VGA::println("[0.000000] Initializing IDT... OK");
    VGA::println("[0.000000] Initializing VGA... OK");
    VGA::println("[0.010200] Keyboard Controller Enabled");
    if (ATA::is_present()) VGA::println("[0.045000] ATA PIO Drive Detected");
    VGA::println("[0.050000] VFS Ramdisk initialized");
    VGA::println("[0.080000] Network Stack Booted");
}

void Commands::cmd_sleep_cmd(const char* args) {
    if (!args[0]) { VGA::println("Usage: sleep <milliseconds>"); return; }
    // A very naive atoi
    uint32_t val = 0;
    while (*args >= '0' && *args <= '9') {
        val = val * 10 + (*args - '0');
        args++;
    }
    sleep(val); // this sleep is defined globally as `nop` loop
}

void Commands::cmd_touch(const char* args) {
    if (!args[0]) { VGA::println("Usage: touch <filename>"); return; }
    if (VFS::create_file(args, false)) {
        VGA::println("File created.");
        VFS::save_to_disk();
    } else {
        VGA::println("Error: Creation failed or file already exists.");
    }
}

void Commands::cmd_mkdir(const char* args) {
    if (!args[0]) { VGA::println("Usage: mkdir <folder_name>"); return; }
    if (VFS::create_file(args, true)) {
        VGA::print("Directory created: "); VGA::println(args);
        VFS::save_to_disk();
    } else {
        VGA::println("Error: No space left or name already exists!");
    }
}

void Commands::cmd_ls() {
    VFile* files = VFS::get_all_files();
    bool found = false;
    VGA::println(" Name                       Type    Size");
    VGA::println(" -----------------------------------------");
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].exists) {
            found = true;
            VGA::print(" ");
            VGA::print(files[i].name);
            int len = 0; while(files[i].name[len]) len++;
            for(int p=len; p<26; p++) VGA::print(" ");
            
            if (files[i].is_dir) VGA::print(" DIR     ");
            else VGA::print(" FILE    ");
            
            char buf[16];
            itoa(files[i].size, buf, 10);
            VGA::println(buf);
        }
    }
    if (!found) VGA::println(" (empty directory)");
}

void Commands::cmd_install() {
    VGA::println("Installing ZilOS to disk...");
    
    if (!ATA::is_present()) {
        VGA::println("  [FAIL] No ATA drive detected! Check Proxmox Bus Type (must be IDE).");
        return;
    }

    char buf[16];
    uint32_t data_sz = sizeof(VFile) * MAX_FILES;
    itoa(data_sz, buf, 10);
    VGA::print("  VFS data size: "); VGA::print(buf); VGA::println(" bytes");
    
    uint32_t sectors = (data_sz / 512) + 1;
    itoa(sectors, buf, 10);
    VGA::print("  Sectors needed: "); VGA::println(buf);
    
    if (VFS::save_to_disk()) {
        VGA::println("  [OK] Data written to disk successfully!");
    } else {
        VGA::println("  [FAIL] Write failed! Drive exists but is not responding.");
    }
}

void Commands::cmd_save() {
    VGA::print("Saving VFS to disk... ");
    if (VFS::save_to_disk()) {
        VGA::println("[OK]");
    } else {
        VGA::println("[FAIL] - ATA drive not detected.");
    }
}

void Commands::cmd_cat(const char* args) {
    if (!args[0]) { VGA::println("Usage: cat <file_name>"); return; }
    VFile* f = VFS::get_file(args);
    if (f) {
        if (f->is_dir) {
            VGA::println("Error: Is a directory");
        } else {
            VGA::println(f->content);
        }
    } else {
        VGA::println("Error: File not found");
    }
}

void Commands::cmd_rm(const char* args) {
    if (!args[0]) { VGA::println("Usage: rm <name>"); return; }
    if (VFS::delete_file(args)) {
        VGA::println("File/Folder removed!");
        VFS::save_to_disk();
    } else {
        VGA::println("Error: File not found");
    }
}

void Commands::cmd_edit(const char* args) {
    if (!args[0]) { VGA::println("Usage: edit <file_name>"); return; }
    Editor::start(args);
}

void Commands::cmd_cpu() {
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(0));
    
    char vendor[13];
    vendor[0] = ebx & 0xFF;
    vendor[1] = (ebx >> 8) & 0xFF;
    vendor[2] = (ebx >> 16) & 0xFF;
    vendor[3] = (ebx >> 24) & 0xFF;
    vendor[4] = edx & 0xFF;
    vendor[5] = (edx >> 8) & 0xFF;
    vendor[6] = (edx >> 16) & 0xFF;
    vendor[7] = (edx >> 24) & 0xFF;
    vendor[8] = ecx & 0xFF;
    vendor[9] = (ecx >> 8) & 0xFF;
    vendor[10] = (ecx >> 16) & 0xFF;
    vendor[11] = (ecx >> 24) & 0xFF;
    vendor[12] = '\0';
    
    VGA::print("CPU Vendor String: ");
    VGA::println(vendor);
}

void Commands::cmd_memory() {
    char buf[16];
    itoa(system_ram_mb, buf, 10);
    
    VGA::println("RAM Information (Read via GRUB Multiboot):");
    VGA::print("Total Physical Memory : ");
    VGA::print(buf);
    VGA::println(" MB");
}

void Commands::cmd_storage() {
    char buf[16];
    size_t used = VFS::get_used_storage();
    itoa(used, buf, 10);
    VGA::println("Storage Information (VFS RAM Disk):");
    VGA::print("Used Storage: ");
    VGA::print(buf);
    VGA::println(" bytes");
    
    itoa(MAX_FILES * (sizeof(VFile)), buf, 10);
    VGA::print("Total VFS Capacity: ");
    VGA::print(buf);
    VGA::println(" bytes");
}

// ---- Network Commands ----

// Helper for parsing lines from config.txt
static void parse_config_line(const char* line, const char* key, uint8_t* ip_out) {
    int kl = 0; while(key[kl]) kl++;
    
    // check prefix
    for(int i=0; i<kl; i++) {
        if (line[i] != key[i]) return;
    }
    
    char val[32];
    int p = 0;
    int i = kl;
    while(line[i] && line[i] != '\n' && line[i] != '\r' && p < 31) {
        val[p++] = line[i++];
    }
    val[p] = '\0';
    ip_parse(val, ip_out);
}

void Commands::cmd_net(const char* args) {
    if (!args[0]) {
        VGA::println("Usage: net status");
        VGA::println("       net apply");
        return;
    }

    if (strcmp(args, "status")) {
        VFile* f = VFS::get_file("network/config.txt");
        if (f) {
            VGA::println("--- Content of /network/config.txt ---");
            VGA::println(f->content);
        } else {
            VGA::println("Error: /network/config.txt not found.");
        }
        VGA::println("--- Current Effective State ---");
        cmd_ifconfig();
    } else if (strcmp(args, "apply")) {
        VFile* f = VFS::get_file("network/config.txt");
        if (!f) {
            VGA::println("Fail: /network/config.txt does not exist!");
            return;
        }

        if (!RTL8139::is_available()) {
            VGA::println("Fail: No Network Interface found.");
            return;
        }
        
        // Zero out current config temporarily
        net_config.configured = false;
        
        char lineBuf[64];
        int pos = 0;
        size_t i = 0;
        bool is_dhcp = false;
        
        while (i < f->size) {
            char c = f->content[i++];
            if (c == '\n' || c == '\r' || i == f->size) {
                if (i == f->size && c != '\n' && c != '\r') lineBuf[pos++] = c;
                lineBuf[pos] = '\0';
                
                if (strcmp(lineBuf, "method=dhcp")) is_dhcp = true;
                else {
                    parse_config_line(lineBuf, "ip=", net_config.ip);
                    parse_config_line(lineBuf, "gateway=", net_config.gateway);
                    parse_config_line(lineBuf, "subnet=", net_config.subnet);
                }
                pos = 0;
                if (c == '\r' && f->content[i] == '\n') i++; // skip CRLF
            } else {
                if (pos < 63) lineBuf[pos++] = c;
            }
        }
        
        if (is_dhcp) {
            VGA::println("Executing DHCP Discovery...");
            if (DHCP::discover()) {
                VGA::println("DHCP Apply Successful.");
            } else {
                VGA::println("DHCP Apply Failed.");
            }
        } else {
            VGA::println("Static IP configuration applied.");
            net_config.configured = true;
        }
    } else {
        VGA::println("Unknown subcommand.");
    }
}

void Commands::cmd_ifconfig() {
    if (!RTL8139::is_available()) {
        VGA::println("No network interface available.");
        return;
    }

    VGA::println("eth0 (RTL8139):");
    
    // MAC
    VGA::print("  MAC Address : ");
    uint8_t mac[6];
    RTL8139::get_mac(mac);
    char hex[] = "0123456789ABCDEF";
    for (int i = 0; i < 6; i++) {
        char buf[3] = {hex[mac[i] >> 4], hex[mac[i] & 0xF], 0};
        VGA::print(buf);
        if (i < 5) VGA::print(":");
    }
    VGA::println("");

    if (!net_config.configured) {
        VGA::println("  Status      : DHCP not configured");
        return;
    }

    char ip_str[16];
    ip_to_str(net_config.ip, ip_str);
    VGA::print("  IP Address  : "); VGA::println(ip_str);
    ip_to_str(net_config.subnet, ip_str);
    VGA::print("  Subnet Mask : "); VGA::println(ip_str);
    ip_to_str(net_config.gateway, ip_str);
    VGA::print("  Gateway     : "); VGA::println(ip_str);
    ip_to_str(net_config.dns, ip_str);
    VGA::print("  DNS Server  : "); VGA::println(ip_str);
}

void Commands::cmd_ping(const char* args) {
    if (!args[0]) { VGA::println("Usage: ping <ip_address>"); return; }
    if (!net_config.configured) { VGA::println("Error: Network not configured"); return; }

    uint8_t target_ip[4];
    if (!ip_parse(args, target_ip)) {
        VGA::println("Error: Invalid IP address");
        return;
    }

    VGA::print("PING "); VGA::print(args); VGA::println(" ...");

    for (int i = 0; i < 4; i++) {
        // Build ICMP echo request
        uint8_t icmp[64];
        icmp[0] = 8;  // Echo request
        icmp[1] = 0;  // Code
        icmp[2] = 0; icmp[3] = 0; // Checksum placeholder
        icmp[4] = 0; icmp[5] = 1; // Identifier
        icmp[6] = 0; icmp[7] = (uint8_t)(i + 1); // Sequence

        // Fill data
        for (int d = 8; d < 64; d++) icmp[d] = (uint8_t)d;

        // Calculate checksum
        uint16_t cksum = IPv4::checksum(icmp, 64);
        icmp[2] = cksum & 0xFF;
        icmp[3] = (cksum >> 8) & 0xFF;

        IPv4::send(target_ip, IP_PROTO_ICMP, icmp, 64);
        IPv4::icmp_received = false;

        // Wait for reply by polling
        bool got_reply = false;
        for (int t = 0; t < 200; t++) {
            for (int k = 0; k < 20000; k++) Ethernet::poll();
            if (IPv4::icmp_received) {
                got_reply = true;
                break;
            }
        }

        VGA::print("64 bytes from "); VGA::print(args);
        VGA::println(got_reply ? ": icmp_seq OK" : ": timeout");
    }
}

void Commands::cmd_dns(const char* args) {
    if (!args[0]) { VGA::println("Usage: dns <hostname>"); return; }
    if (!net_config.configured) { VGA::println("Error: Network not configured"); return; }

    uint8_t ip[4];
    VGA::print("Resolving "); VGA::print(args); VGA::println("...");

    if (DNS::resolve(args, ip)) {
        char ip_str[16];
        ip_to_str(ip, ip_str);
        VGA::print(args); VGA::print(" → "); VGA::println(ip_str);
    } else {
        VGA::println("Error: DNS resolution failed");
    }
}

void Commands::cmd_wget(const char* args) {
    if (!args[0]) { VGA::println("Usage: wget <url> <filename>"); return; }
    if (!net_config.configured) { VGA::println("Error: Network not configured"); return; }

    // Parse "url filename" from args
    char url[256] = {0};
    char filename[32] = {0};

    int i = 0;
    int u = 0;
    while (args[i] && args[i] != ' ') {
        url[u++] = args[i++];
    }
    url[u] = '\0';

    if (args[i] == ' ') {
        i++;
        int f = 0;
        while (args[i]) {
            filename[f++] = args[i++];
        }
        filename[f] = '\0';
    }

    if (!url[0] || !filename[0]) {
        VGA::println("Usage: wget <url> <filename>");
        return;
    }

    HTTP::get(url, filename);
}

void Commands::cmd_reboot() {
    VGA::println("Saving VFS to disk...");
    VFS::save_to_disk();
    VGA::println("Rebooting system...");
    outb(0x64, 0xFE);
}

void Commands::cmd_shutdown() {
    VGA::println("Saving VFS to disk...");
    VFS::save_to_disk();
    VGA::println("Attempting shutdown...");
    outw(0x604, 0x2000);
}
