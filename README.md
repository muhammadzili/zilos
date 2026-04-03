# 🐧 ZilOS Test 1.0 (CLI Edition)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform: i386](https://img.shields.io/badge/Platform-i386-blue.svg)](#)
[![Status: Testing](https://img.shields.io/badge/Status-Test--1.0-orange.svg)](#)

**ZilOS** is a lightweight, high-performance, and persistent Command Line Operating System built from scratch for the x86 architecture. This version (**Test 1.0**) focuses on establishing a stable core for CLI-based hobbyist computing with integrated networking and filesystem persistence.

---

## 💻 System Requirements

| Component | Minimum Requirement | Recommended |
| :--- | :--- | :--- |
| **CPU** | i386 (Pentium / Single Core) | Intel VT-x / AMD-V enabled |
| **RAM** | 32 MB | 64 MB+ |
| **Storage** | 64 MB (IDE/ATA Interface) | 128 MB (for /home data) |
| **Network** | Realtek RTL8139 NIC | RTL8139 + DHCP Server |
| **Display** | VGA Text Mode (80x25) | Standard VGA Monitor |

---

## ✨ Key Features

- **💻 Pure CLI Architecture**: Operates in standard 80x25 VGA text mode, maximizing performance and compatibility.
- **🌐 Network Stack & DHCP**: Auto-discovery of network settings via RTL8139 with manual configuration support.
- **💾 VFS Persistence**: Custom Virtual File System that synchronizes data to `zilos.img` via ATA drivers.
- **🛡️ CLI Stability**: Enforced White-on-Black color scheme for readability and reduced eye strain.
- **🛠️ UNIX-like Suite**: Standard toolbox including `ls`, `mkdir`, `touch`, `rm`, `cat`, `uname`, `uptime`, and `edit`.
- **🚀 OS Installer**: Professional BIOS-style setup utility with automatic partitioning and system extraction.

---

## 🛠️ Build & Installation

### 1. Build Requirements
Ensure you have the following tools installed:
- `g++` (cross-compiler for i386)
- `nasm`
- `qemu-system-i386` (for testing)
- `xorriso` & `grub-mkrescue` (for ISO generation)

### 2. Compilation
```bash
# Clone and enter directory
git clone https://github.com/muhammadzili/zilos.git
cd zilos

# Compile and create ISO
make iso
```

### 3. Running ZilOS
```bash
# Run with standard storage
make run

# Run with full networking (Recommended)
make run-net
```

---

## 📊 Technical Details
- **Binary Size**: ~12 MB (ISO)
- **Kernel Type**: Monolithic (Minimalist)
- **Filesystem**: Custom RAM-cached ATA VFS
- **Language**: C++ (Freestanding) & Assembly (NASM)

## 🤝 Contribution
ZilOS is an open-source hobby project. Feel free to fork the repository, open issues, or submit pull requests to help improve the kernel, drivers, or system utilities.

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---
*ZilOS Test 1.0 - Built for the metal.*
