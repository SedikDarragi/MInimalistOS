# Minimalist OS - Build Guide

This guide provides instructions for building and running the Minimalist OS kernel.

## Prerequisites

- Linux-based system (tested on Ubuntu 20.04/22.04)
- GCC cross-compiler for i386-elf target
- NASM assembler
- QEMU for emulation
- GNU Make

## Installation

### On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y build-essential nasm qemu-system-x86
```

## Building the Kernel

### Using the Makefile (Recommended)
```bash
# Clean previous builds
make clean

# Build the kernel
make

# Create bootable disk image
make os.img

# Run in QEMU
make run
```

### Using the Simple Build Script
For troubleshooting or quick builds, you can use the simple build script:
```bash
chmod +x simple_build.sh
./simple_build.sh
```

## Project Structure

```
/
├── boot/           # Bootloader code
├── drivers/        # Device drivers
│   └── vga.c       # VGA text mode driver
├── fs/             # File system implementation
├── include/        # Header files
├── kernel/         # Kernel source code
│   ├── entry.s     # Kernel entry point (assembly)
│   ├── idt.c       # Interrupt Descriptor Table
│   ├── idt_asm.asm # IDT assembly routines
│   ├── isr.c       # Interrupt Service Routines
│   └── ...
├── Makefile        # Build configuration
└── simple_build.sh # Alternative build script
```

## Running in QEMU

### Basic QEMU Command
```bash
qemu-system-i386 -m 32M -drive file=os.img,format=raw,if=ide -vga std -display sdl
```

### Common QEMU Options
- `-m 32M`: Allocate 32MB of RAM
- `-smp 1`: Use 1 CPU core
- `-serial stdio`: Redirect serial output to terminal
- `-no-reboot`: Exit on kernel panic
- `-d int`: Log interrupts (for debugging)

## Debugging

### GDB with QEMU
1. Start QEMU in debug mode:
   ```bash
   qemu-system-i386 -s -S -m 32M -drive file=os.img,format=raw,if=ide
   ```
2. In another terminal, connect with GDB:
   ```bash
   gdb -ex "target remote localhost:1234" -ex "symbol-file kernel.elf"
   ```

## Common Issues

### Missing Dependencies
If you get build errors about missing headers or tools, ensure all prerequisites are installed.

### Linker Errors
If you see undefined references, check that all object files are being linked and in the correct order.

### QEMU Issues
If QEMU doesn't start, try different display backends (`-display sdl` or `-display gtk`).

## License
[Your License Here]
