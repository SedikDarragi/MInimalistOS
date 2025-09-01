# MinimalOS

A minimalist operating system written in C and Assembly with kernel, bootloader, and modern features.

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Build Status](https://github.com/YOUR_USERNAME/MinimalOS/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/MinimalOS/actions)

## Features

- **Custom Bootloader**: 16-bit to 32-bit transition with GDT setup
- **Kernel**: 32-bit protected mode kernel with memory management
- **Shell System**: Interactive command-line interface with built-in commands
- **File Manager**: Basic filesystem with file operations
- **Network Stack**: TCP/IP stack with ping and network configuration
- **Graphical UI**: Desktop environment with windows and applications

## Getting Started

### Prerequisites

- GCC with 32-bit support
- NASM (Netwide Assembler)
- GNU Make
- QEMU (for testing)

### Installation

#### Arch Linux
```bash
sudo pacman -S --needed base-devel qemu-full nasm
```

#### Ubuntu/Debian
```bash
sudo apt-get install gcc-multilib nasm make qemu-system-x86
```

### Building

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/MinimalOS.git
cd MinimalOS

# Build the OS
make

# Run in QEMU
make run

# Run with VNC (view at localhost:5901)
make run-vnc

# Run with debug output
make run-debug
```

## Project Structure

```
MinimalOS/
├── boot/           # Bootloader
│   └── boot.asm    # 16-bit bootloader
├── kernel/         # Kernel core
│   ├── start.asm   # Assembly entry point
│   ├── kernel.c    # Main kernel
│   ├── kernel.h    # Kernel headers
│   ├── shell.c     # Shell implementation
│   ├── shell.h     # Shell headers
│   └── utils.c     # Utility functions
├── drivers/        # Hardware drivers
│   ├── vga.c       # VGA text mode driver
│   ├── vga.h       # VGA headers
│   ├── keyboard.c  # Keyboard driver
│   ├── keyboard.h  # Keyboard headers
│   ├── timer.c     # Timer driver
│   └── timer.h     # Timer headers
├── fs/             # Filesystem
│   ├── filesystem.c # File operations
│   └── filesystem.h # Filesystem headers
├── net/            # Network stack
│   ├── network.c   # Network implementation
│   └── network.h   # Network headers
├── ui/             # User interface
│   ├── ui.c        # GUI implementation
│   └── ui.h        # UI headers
├── .gitignore      # Git ignore file
├── Makefile        # Build configuration
├── link.ld         # Linker script
└── README.md       # This file
```

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [OSDev Wiki](https://wiki.osdev.org/) for invaluable OS development resources
- [BrokenThorn Entertainment](http://www.brokenthorn.com/Resources/) for OS development tutorials
- [JamesM's kernel development tutorials](http://www.jamesmolloy.co.uk/tutorial_html/)

## Limitations

This is a educational/demonstration OS with the following limitations:

- No multitasking or process management
- Simple memory allocator (no free)
- Basic filesystem (in-memory only)
- Limited network functionality (simulation)
- Text-mode GUI only
- No disk I/O beyond bootloader

## Future Enhancements

- Real filesystem with disk I/O
- Multitasking and process scheduling
- Advanced memory management
- Real network driver integration
- Graphics mode support
- More applications and utilities

## License

This project is for educational purposes and should be used for commercial reasons without my permission first , Feel free to use and modify as needed.
