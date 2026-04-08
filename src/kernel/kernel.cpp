// standard / boot headers
#include "boot/multiboot.hpp"
#include "arch/i386/gdt.hpp"
#include "arch/i386/idt.hpp"
#include "arch/i386/interrupts.hpp"
#include "arch/i386/timer.hpp"

// hardware drivers
#include "drivers/storage/ata.hpp"
#include "drivers/input/keyboard.hpp"
#include "drivers/net/rtl8139.hpp"
#include "drivers/video/vga.hpp"

// hal, fs & net
#include "hal/hal.hpp"
#include "fs/config.hpp"
#include "fs/vfs.hpp"
#include "net/arp.hpp"
#include "net/dhcp.hpp"
#include "net/netutils.hpp"
#include "net/tcp.hpp"

// user space
#include "user/apps/installer.hpp"
#include "user/cli/commands.hpp"
#include "user/cli/shell.hpp"

#include "kernel/scheduler.hpp"
#include "kernel/memory.hpp"

uint32_t system_ram_mb = 64;

extern "C" void kernel_main(uint32_t magic, multiboot_info* mbd) {
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        system_ram_mb = (mbd->mem_lower + mbd->mem_upper) / 1024;
    }
    
    GDT::initialize();
    IDT::initialize();
    VGA::initialize(); 
    
    VGA::println("ZilOS Test 1.0 - CLI Kernel");
    VGA::println("GDT and IDT active.");
    
    Keyboard::initialize();
    
    static ATA ata_driver;
    if (ATA::init()) {
        HAL::set_block_device(&ata_driver);
        VGA::println("ATA: Drive detected and registered to HAL.");
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
    
    for (uint32_t i = 0; i < 50000000; i++) asm volatile("nop");

    if (!Config::is_installed()) {
        VGA::clear();
        Installer::run();
        Config::init();
    }
    
    VGA::println("Loading network configuration...");
    Commands::cmd_net("apply");

    Memory::PMM::initialize(system_ram_mb * 1024);
    Memory::Heap::initialize();
    initialize();
    
    Timer::initialize(100);
    Timer::set_handler(tick);

    VGA::clear();
    VGA::println("ZilOS successfully booted.");
    Shell::init();
    Shell::run();

    while (true) {
        asm volatile("hlt");
    }
}
