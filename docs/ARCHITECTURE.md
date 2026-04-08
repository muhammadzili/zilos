# ZilOS Architecture

## Overview

ZilOS is a hobby x86 (i386) operating system kernel written in C++.

## Components

### Kernel Core (`src/kernel/`)
- `kernel.cpp` - Main kernel entry point
- `memory.cpp` - Memory management
- `scheduler.cpp` - Process scheduler
- `panic.cpp` - Kernel panic handling

### Architecture (`src/arch/i386/`)
- `boot.s` - Boot sector (assembly)
- `gdt.cpp/s` - Global Descriptor Table
- `idt.cpp/s` - Interrupt Descriptor Table
- `paging.cpp` - Memory paging
- `timer.cpp` - Programmable Interval Timer
- `interrupts.cpp/s` - Interrupt handling

### Drivers (`src/drivers/`)
- **Storage**: ATA disk driver
- **Network**: RTL8139 Ethernet driver
- **Video**: VGA display driver
- **Input**: Keyboard driver
- **Bus**: PCI bus driver

### File System (`src/fs/`)
- `vfs.cpp` - Virtual File System
- `config.cpp` - Configuration management

### Networking (`src/net/`)
- `ethernet.cpp` - Ethernet frame handling
- `arp.cpp` - Address Resolution Protocol
- `ipv4.cpp` - IPv4 protocol
- `tcp.cpp` / `udp.cpp` - Transport protocols
- `dhcp.cpp` - DHCP client
- `dns.cpp` - DNS resolver

### User Space (`src/user/`)
- `cli/` - Command-line interface & shell
- `apps/` - Applications (editor, installer)
- `net/` - Network utilities (HTTP client)

### Hardware Abstraction (`src/hal/`)
- `hal.cpp` - Hardware abstraction interface
