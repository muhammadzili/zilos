#include "drivers/net/rtl8139.hpp"
#include "utils/ports.hpp"
#include "drivers/video/vga.hpp"

// rtl8139 registers
#define REG_MAC0        0x00
#define REG_RBSTART     0x30
#define REG_CMD         0x37
#define REG_CAPR        0x38
#define REG_IMR         0x3C
#define REG_ISR         0x3E
#define REG_TCR         0x40
#define REG_RCR         0x44
#define REG_CONFIG1     0x52
#define REG_TSAD0       0x20
#define REG_TSD0        0x10

// cmd bits
#define CMD_RESET       0x10
#define CMD_RX_ENABLE   0x08
#define CMD_TX_ENABLE   0x04

// rx buffer size for 8k+16 mode idc about the extra padding
#define RX_ARRAY_SIZE (8192 + 16 + 1500)

// static members
PCIDevice RTL8139::dev;
uint16_t RTL8139::io_base = 0;
uint8_t RTL8139::mac_addr[6] = {0};
uint8_t RTL8139::rx_buffer[RX_ARRAY_SIZE] __attribute__((aligned(4096)));
uint32_t RTL8139::rx_offset = 0;
uint8_t RTL8139::tx_cur = 0;
bool RTL8139::available = false;

static uint8_t tx_buffers[4][1536] __attribute__((aligned(4)));

bool RTL8139::is_available() { return available; }

void RTL8139::get_mac(uint8_t* mac_out) {
    for (int i = 0; i < 6; i++) mac_out[i] = mac_addr[i];
}

// driver_print_hex: helper for debug idc
static void driver_print_hex(uint32_t val) {
    char hex[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        char c[2] = {hex[(val >> (i * 4)) & 0xF], 0};
        VGA::print(c);
    }
}

// init: find pci device and config the nic fr
bool RTL8139::init() {
    if (!PCI::find_device(RTL8139_VENDOR, RTL8139_DEVICE, &dev)) {
        VGA::println("[NIC] Hardware not detected.");
        available = false;
        return false;
    }

    uint32_t bar0 = PCI::read(dev.bus, dev.slot, dev.func, 0x10);
    io_base = bar0 & 0xFFFC;
    
    // enable pci bus master and io space ts is mandatory
    uint32_t pci_cmd = PCI::read(dev.bus, dev.slot, dev.func, 0x04);
    pci_cmd |= 0x05; 
    PCI::write(dev.bus, dev.slot, dev.func, 0x04, pci_cmd);

    // power on and reset
    outb(io_base + REG_CONFIG1, 0x00);
    outb(io_base + REG_CMD, CMD_RESET);
    for (volatile int i = 0; i < 100000; i++) {
        if (!(inb(io_base + REG_CMD) & CMD_RESET)) break;
    }

    // read hardcoded mac from rom
    for (int i = 0; i < 6; i++) mac_addr[i] = inb(io_base + REG_MAC0 + i);

    // setup 8k rx buffer
    outl(io_base + REG_RBSTART, (uint32_t)rx_buffer);
    outw(io_base + REG_CAPR, 0x0000); 
    rx_offset = 0;

    // reset isr/imr: disable irqs cuz we poll fr
    outw(io_base + REG_IMR, 0x0000); 
    outw(io_base + REG_ISR, 0xFFFF);

    // config rx: 8k+16 mode
    // accept aap|apm|am|ab = 0x0f (promiscuous idc)
    outl(io_base + REG_RCR, 0x0000000F); 

    // kickstart rx and tx
    outb(io_base + REG_CMD, CMD_RX_ENABLE | CMD_TX_ENABLE);

    available = true;
    return true;
}

// send_packet: copy data to tx buffer and fire idgaf if it drops
bool RTL8139::send_packet(const void* data, uint16_t len) {
    if (!available) return false;

    // copy to physical buffer
    const uint8_t* src = (const uint8_t*)data;
    for (uint16_t i = 0; i < len; i++) tx_buffers[tx_cur][i] = src[i];

    outl(io_base + REG_TSAD0 + (tx_cur * 4), (uint32_t)tx_buffers[tx_cur]);
    outl(io_base + REG_TSD0 + (tx_cur * 4), (uint32_t)len);

    // wait for tx completion status
    for (volatile int i = 0; i < 100000; i++) {
        if (inl(io_base + REG_TSD0 + (tx_cur * 4)) & (1 << 15)) break;
    }

    tx_cur = (tx_cur + 1) % 4; // cyclically update tx desc
    return true;
}

// receive_packet: poll rx buffer and copy packet if available
int RTL8139::receive_packet(void* buffer, uint16_t max_len) {
    if (!available) return -1;

    // fast check if buffer is empty
    if (inb(io_base + REG_CMD) & 0x01) return 0;

    uint16_t status = *(uint16_t*)(rx_buffer + rx_offset);
    uint16_t length = *(uint16_t*)(rx_buffer + rx_offset + 2);

    // check rok bit; ts means packet is ok
    if (!(status & 0x01)) return 0;

    uint16_t packet_len = length - 4; // minus crc
    if (packet_len > max_len) packet_len = max_len;

    uint8_t* dst = (uint8_t*)buffer;
    for (uint16_t i = 0; i < packet_len; i++) {
        dst[i] = rx_buffer[(rx_offset + 4 + i) % RX_ARRAY_SIZE];
    }

    // update pointers and wrap at 8k fr
    rx_offset = (rx_offset + length + 4 + 3) & ~3;
    rx_offset %= 8192; 
    outw(io_base + REG_CAPR, (uint16_t)(rx_offset - 16));

    // clear rok interrupt status
    outw(io_base + REG_ISR, 0x0001);

    return packet_len;
}
