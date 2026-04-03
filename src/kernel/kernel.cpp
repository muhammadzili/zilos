#include "drivers/vga.hpp"
#include "drivers/keyboard.hpp"
#include "drivers/ata.hpp"
#include "kernel/gdt.hpp"
#include "kernel/idt.hpp"
#include "kernel/interrupts.hpp"
#include "cli/shell.hpp"
#include "fs/vfs.hpp"
#include "fs/config.hpp"
#include "boot/multiboot.hpp"
#include "drivers/rtl8139.hpp"
#include "net/netutils.hpp"
#include "net/arp.hpp"
#include "net/tcp.hpp"
#include "net/dhcp.hpp"
#include "apps/installer.hpp"
#include "cli/commands.hpp"

uint32_t system_ram_mb = 64;

extern "C" void kernel_main(uint32_t magic, multiboot_info* mbd) {
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        system_ram_mb = (mbd->mem_lower + mbd->mem_upper) / 1024;
    }
    
    // 1. Initial State
    GDT::initialize();
    IDT::initialize();
    VGA::initialize(); 
    
    VGA::println("ZilOS Test 1.0 - CLI Kernel");
    VGA::println("GDT and IDT active.");
    
    // 2. Hardware Drivers
    Keyboard::initialize();
    
    if (ATA::init()) {
        VGA::println("ATA: Drive detected.");
    }
    
    VFS::init();
    Config::init();
    ARP::init();
    TCP::init();

    VGA::println("Initializing networking hardware...");
    if (RTL8139::init()) {
        uint8_t mac[6];
        RTL8139::get_mac(mac);
        net_memcpy(net_config.mac, mac, 6);
        VGA::println("NIC: RTL8139 ready.");
    }
    
    asm volatile("sti");
    
    // Brief delay to let the user see the boot sequence
    for (uint32_t i = 0; i < 50000000; i++) asm volatile("nop");

    // ── Boot Decision ──────────────────────────

    if (!Config::is_installed()) {
        VGA::clear();
        Installer::run();
        Config::init(); // Re-read configs after install
    }
    
    // After VFS and/or Installer finished: Load network from config.txt
    VGA::println("Loading network configuration...");
    Commands::cmd_net("apply");

    VGA::clear();
    VGA::println("ZilOS successfully booted.");
    Shell::init();
    Shell::run();

    while (true) {
        asm volatile("hlt");
    }
}
