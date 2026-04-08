# ZilOS Documentation

## Project Structure

```
zilos/
├── src/                    # Source code
│   ├── arch/              # Architecture-specific code (i386)
│   ├── drivers/           # Device drivers
│   ├── fs/                # File system
│   ├── hal/               # Hardware Abstraction Layer
│   ├── include/           # Header files
│   ├── kernel/            # Kernel core
│   ├── net/               # Networking stack
│   └── user/              # User-space applications
├── build/                 # Build output (generated)
├── docs/                  # Documentation
├── tests/                 # Test suite
├── linker.ld              # Linker script
└── Makefile               # Build system
```

## Building

```bash
make          # Build zilos.bin and zilos.img
make clean    # Clean build artifacts
make iso      # Create bootable ISO
make run      # Run in QEMU
make run-net  # Run with network support
```

## Development

See [CONTRIBUTING.md](../CONTRIBUTING.md) for development guidelines.
