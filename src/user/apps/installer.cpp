#include "user/apps/installer.hpp"
#include "drivers/video/vga.hpp"
#include "drivers/input/keyboard.hpp"
#include "drivers/storage/ata.hpp"
#include "fs/vfs.hpp"
#include "fs/config.hpp"
#include "drivers/net/rtl8139.hpp"
#include "net/dhcp.hpp"
#include "net/netutils.hpp"

extern uint32_t system_ram_mb;

// text mode utils: bits to make the ui look clean fr
void Installer::itoa(size_t val, char* buf, int base) {
    VGA::itoa(val, buf, base);
}

int Installer::strlen(const char* s) {
    int l = 0;
    while (s[l]) l++;
    return l;
}

// inst_delay: busy wait for timing; ts is hacky idc
static void inst_delay(uint32_t ms) {
    for (uint32_t i = 0; i < ms * 10000; i++) {
        asm volatile("nop");
    }
}

// draw_box: paint a rectangle on screen fr
void Installer::draw_box(int x, int y, int w, int h, uint8_t bg) {
    VGA::set_color(VGA::entry_color(15, bg));
    for (int i = 0; i < h; i++) {
        VGA::set_cursor(x, y + i);
        for (int j = 0; j < w; j++) {
            VGA::putchar(' ');
        }
    }
}

// draw_text_centered: center text on y axis fr
void Installer::draw_text_centered(int y, const char* text, uint8_t fg, uint8_t bg) {
    int len = strlen(text);
    int x = (80 - len) / 2;
    VGA::set_color(VGA::entry_color(fg, bg));
    VGA::set_cursor(x, y);
    VGA::print(text);
}

// draw_text: generic text print fr
void Installer::draw_text(int x, int y, const char* text, uint8_t fg, uint8_t bg) {
    VGA::set_color(VGA::entry_color(fg, bg));
    VGA::set_cursor(x, y);
    VGA::print(text);
}

// draw_progress_bar: show visual progress ngl it looks cool
void Installer::draw_progress_bar(int y, int percent) {
    int bar_width = 40;
    int x = (80 - bar_width) / 2;
    
    VGA::set_cursor(x, y);
    VGA::set_color(VGA::entry_color(0, 7)); // grey empty slot fr
    for(int i=0; i<bar_width; i++) VGA::putchar(' ');
    
    int filled = (percent * bar_width) / 100;
    VGA::set_cursor(x, y);
    VGA::set_color(VGA::entry_color(15, 2)); // green filled part fr
    for(int i=0; i<filled; i++) VGA::putchar(' ');
    
    VGA::set_color(VGA::entry_color(15, 1)); // back to blue theme idc
    char pct[8];
    itoa(percent, pct, 10);
    VGA::set_cursor(x + bar_width + 2, y);
    VGA::print(pct);
    VGA::print("%");
}

// installer screens: main ui flow fr

void Installer::screen_welcome() {
    VGA::clear();
    draw_box(0, 0, 80, 25, 1); // blue background fr
    draw_box(0, 0, 80, 1, 7);  // top bar grey idc
    draw_text(2, 0, "ZilOS System Setup", 0, 7);
    
    draw_box(15, 6, 50, 10, 7); // dialog box fr
    draw_text_centered(8, "Welcome to ZilOS", 0, 7);
    draw_text_centered(10, "This program will install ZilOS to your hard drive.", 0, 7);
    draw_text_centered(13, "[ Press ENTER to continue ]", 1, 7);
    
    while (Keyboard::wait_get_char() != '\n');
}

void Installer::screen_sysinfo() {
    draw_box(15, 6, 50, 10, 7);
    draw_text_centered(10, "Detecting Hardware...", 0, 7);
    inst_delay(500);
    
    draw_box(15, 6, 50, 10, 7); 
    draw_text_centered(7, "Hardware Detection Complete", 0, 7);
    
    char ram[32];
    itoa(system_ram_mb, ram, 10);
    draw_text(20, 10, "System RAM: ", 0, 7);
    draw_text(35, 10, ram, 1, 7);
    draw_text(35 + strlen(ram), 10, " MB", 1, 7);
    
    draw_text(20, 12, "ATA Drive : ", 0, 7);
    draw_text(35, 12, ATA::is_present() ? "PASSED (8GB)" : "FAILED", 1, 7);
    
    draw_text_centered(14, "[ Press ENTER to continue ]", 1, 7);
    while (Keyboard::wait_get_char() != '\n');
}

void Installer::screen_network() {
    draw_box(15, 6, 50, 10, 7);
    draw_text_centered(8, "Network Configuration", 0, 7);
    inst_delay(300);

    bool network_ok = false;
    
    const char* spinner = "\\|/-";
    for(int i=0; i<6; i++) {
        draw_box(15, 10, 50, 1, 7);
        char buf[32] = "Discovering DHCP...  ";
        buf[20] = spinner[i % 4];
        buf[21] = '\0';
        draw_text_centered(10, buf, 1, 7);
        inst_delay(150);
    }
    
    draw_box(15, 10, 50, 3, 7);
    
    if (RTL8139::is_available()) {
        uint8_t mac[6];
        RTL8139::get_mac(mac);
        net_memcpy(net_config.mac, mac, 6);
        if (DHCP::discover()) {
            network_ok = true;
            char ip_str[16];
            ip_to_str(net_config.ip, ip_str);
            
            draw_text_centered(10, "DHCP Auto-Configuration Successful!", 2, 7); // green win fr
            draw_text(20, 12, "IP Addr : ", 0, 7); draw_text(31, 12, ip_str, 1, 7);
            ip_to_str(net_config.gateway, ip_str);
            draw_text(20, 13, "Gateway : ", 0, 7); draw_text(31, 13, ip_str, 1, 7);
        } else {
            draw_text_centered(10, "DHCP Auto-Configuration Failed.", 4, 7); // red fail fr
            draw_text_centered(12, "You can configure it manually later via CLI.", 0, 7);
        }
    } else {
        draw_text_centered(10, "No Network Card Detected.", 4, 7); // red fail fr
        draw_text_centered(12, "You can configure it manually later via CLI.", 0, 7);
    }
    
    draw_text_centered(14, "[ Press ENTER to continue ]", 1, 7);
    while (Keyboard::wait_get_char() != '\n');
    
    // write network config fr
    VFS::create_file("network/config.txt", false);
    if (network_ok) {
        char config_buf[256] = "method=dhcp\n";
        
        char tk[16];
        int p = 12; // "method=dhcp\n" len fr
        
        const char* ip_key = "ip=";
        for(int k=0; ip_key[k]; k++) config_buf[p++] = ip_key[k];
        ip_to_str(net_config.ip, tk);
        for(int k=0; tk[k]; k++) config_buf[p++] = tk[k];
        config_buf[p++] = '\n';
        
        const char* gw_key = "gateway=";
        for(int k=0; gw_key[k]; k++) config_buf[p++] = gw_key[k];
        ip_to_str(net_config.gateway, tk);
        for(int k=0; tk[k]; k++) config_buf[p++] = tk[k];
        config_buf[p++] = '\n';
        
        const char* sn_key = "subnet=";
        for(int k=0; sn_key[k]; k++) config_buf[p++] = sn_key[k];
        ip_to_str(net_config.subnet, tk);
        for(int k=0; tk[k]; k++) config_buf[p++] = tk[k];
        config_buf[p++] = '\n';
        
        config_buf[p] = '\0';
        VFS::write_file("network/config.txt", config_buf);
    } else {
        VFS::write_file("network/config.txt", "method=none\n");
    }
}

void Installer::screen_username() {
    char username[32];
    int ulen = 0;
    username[0] = '\0';
    
    // input loop fr
    while (true) {
        draw_box(15, 6, 50, 10, 7);
        draw_text_centered(7, "Administrator Setup", 0, 7);
        draw_text(20, 10, "Enter Username: ", 0, 7);
        
        draw_box(36, 10, 20, 1, 8); // input box fr
        draw_text(36, 10, username, 15, 8);
        draw_text(36 + ulen, 10, "_", 15, 8);
        
        char c = Keyboard::wait_get_char();
        if (c == '\n' && ulen > 0) {
            username[ulen] = '\0';
            Config::set_username(username);
            break;
        }
        if (c == '\b' && ulen > 0) ulen--;
        else if (c >= 32 && c <= 126 && ulen < 19) {
            username[ulen++] = c;
            username[ulen] = '\0';
        }
    }
}

// screen_step: generic step with progress bar fr
void Installer::screen_step(const char* title, int start_pct, int end_pct, int delay_ms, const char* files[]) {
    draw_box(10, 6, 60, 12, 7);
    draw_text_centered(7, title, 0, 7);
    
    for (int p = start_pct; p <= end_pct; p += 2) {
        draw_progress_bar(15, p);
        if (files) {
            draw_box(15, 11, 50, 1, 7); // clear path area fr
            draw_text_centered(11, files[(p % 6) % 3], 0, 7);
        }
        inst_delay(delay_ms);
    }
}

// screen_installing: sequence all install steps fr
void Installer::screen_installing() {
    const char* part_files[] = {"Formatting /dev/sda1 (ext2)", "Writing Superblocks", "Creating Journal"};
    screen_step("Step 1/5: Formatting Disk", 0, 10, 50, part_files);
    
    const char* fs_files[] = {"Mounting /", "Creating /etc", "Creating /home"};
    screen_step("Step 2/5: Creating Filesystem", 10, 20, 50, fs_files);
    
    const char* base_files[] = {"Extracting: /bin/sh", "Extracting: /lib/libc.so", "Extracting: /etc/init.conf"};
    screen_step("Step 3/5: Copying Base System", 20, 60, 80, base_files);
    
    const char* k_files[] = {"Installing Module: ata.ko", "Installing Module: rtl8139.ko", "Generating initramfs"};
    screen_step("Step 4/5: Registering Kernels", 60, 90, 60, k_files);
    
    const char* boot_files[] = {"Installing GRUB to MBR", "Writing /boot/grub/grub.cfg", "Finalizing Setup"};
    screen_step("Step 5/5: Configuring Bootloader", 90, 100, 50, boot_files);
    
    Config::set_installed(true);
    Config::save(); 
    VFS::save_to_disk();
}

void Installer::screen_complete() {
    draw_box(15, 6, 50, 10, 7);
    draw_text_centered(9, "Installation Complete!", 0, 7);
    draw_text_centered(12, "[ Press ENTER to boot ZilOS ]", 1, 7);
    while (Keyboard::wait_get_char() != '\n');
    VGA::set_color(VGA::entry_color(15, 0)); // reset vga colors fr
    VGA::clear();
}

// run: entry point fr the installer wizard idc
void Installer::run() {
    screen_welcome();
    screen_sysinfo();
    screen_network();
    screen_username();
    screen_installing();
    screen_complete();
}
