# Building ZilOS

## Prerequisites

- **Compiler**: `g++` with 32-bit support
- **Assembler**: `nasm`
- **Linker**: `ld`
- **Emulator**: `qemu-system-i386`
- **ISO Tools**: `grub-mkrescue`, `xorriso`

### Ubuntu/Debian
```bash
sudo apt install build-essential nasm qemu grub-common xorriso
```

### Arch Linux
```bash
sudo pacman -S base-devel nasm qemu grub
```

## Build Commands

| Command | Description |
|---------|-------------|
| `make` | Build kernel binary and disk image |
| `make clean` | Remove all build artifacts |
| `make iso` | Create bootable ISO image |
| `make run` | Run in QEMU (disk mode) |
| `make run-net` | Run with network emulation |
| `make run-iso` | Run from ISO image |

## Build Output

- `zilos.bin` - Kernel binary
- `zilos.img` - 64MB disk image
- `zilos.iso` - Bootable ISO
- `build/` - Object files
- `iso/` - ISO structure (temporary)
