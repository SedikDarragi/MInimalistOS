# MinimalistOS

A from-scratch operating system built in C and x86 Assembly, targeting i386 (32-bit protected mode). Runs in QEMU with zero external dependencies.

## Features

- **Custom Bootloader** — 512-byte MBR that transitions from 16-bit real mode to 32-bit protected mode
- **Preemptive Multitasking** — Round-robin scheduler with context switching at 100Hz
- **Memory Management** — Physical page allocator with bitmap, identity-mapped paging, kernel heap
- **System Calls** — INT 0x80 interface with 35 syscalls covering process, file, device, and security operations
- **Interactive Shell** — Built-in commands for process management, file operations, and system info
- **VFS + RAMFS** — In-memory filesystem with create, read, write, and seek support
- **Device Drivers** — VGA text mode, PS/2 keyboard (QWERTY/QWERTZ/AZERTY), PIT timer, serial port, PCI bus
- **ELF Loader** — Parses and executes ELF32 binaries
- **Security Model** — UID/GID-based access control, authentication, and ACLs
- **Power Management** — Simulated battery, thermal monitoring, and CPU throttling
- **Network Stack** — Device abstraction with NE2000 PCI detection and packet buffering (skeleton)
- **Monitoring** — System stats, performance metrics, and multi-level logging

## Prerequisites

- GCC with 32-bit support (`-m32`)
- NASM (Netwide Assembler)
- GNU Make
- GNU ld
- QEMU (`qemu-system-i386`)

### Install Dependencies

<details>
<summary>Arch Linux</summary>

```bash
sudo pacman -S --needed base-devel nasm qemu-full
```
</details>

<details>
<summary>Ubuntu / Debian</summary>

```bash
sudo apt install gcc-multilib nasm make qemu-system-x86
```
</details>

<details>
<summary>Fedora</summary>

```bash
sudo dnf install gcc glibc-devel.i686 nasm make qemu-system-x86
```
</details>

## Building & Running

```bash
make            # Build the OS (creates os.img)
make run        # Launch in QEMU
make run-debug  # Serial debug output in terminal
make run-vnc    # Headless with VNC (connect via vncviewer localhost:1)
```

## Project Structure

```
.
├── boot/
│   └── debug_boot.asm       # Bootloader (real → protected mode)
├── kernel/
│   ├── entry.s              # Assembly entry point (_start)
│   ├── kmain.c              # Kernel initialization
│   ├── idt.c                # Interrupt Descriptor Table
│   ├── interrupts.s         # ISR/IRQ stubs
│   ├── process.c            # Process management & scheduler
│   ├── context.c            # Context switching logic
│   ├── context_switch.s     # Context switch assembly
│   ├── memory.c             # Physical pages, paging, heap
│   ├── syscall.c            # System call dispatcher
│   ├── shell.c              # Interactive shell
│   ├── string.c             # Freestanding string library
│   ├── log.c                # Multi-level logging
│   ├── device.c             # Device abstraction layer
│   ├── program_loader.c     # ELF32 loader
│   ├── security.c           # Users, permissions, ACLs
│   ├── monitor.c            # System monitoring
│   ├── power.c              # Power management (simulated)
│   ├── network.c            # Network stack
│   ├── net_core.c           # Network device registry
│   ├── pci.c                # PCI bus scanning
│   ├── usermode.c           # Ring 3 transition
│   └── *_test.c             # Subsystem test files
├── drivers/
│   ├── vga.c                # VGA text-mode driver
│   ├── keyboard.c           # PS/2 keyboard driver
│   ├── keyboard_intl.c      # International layouts
│   ├── timer.c              # PIT timer (100Hz)
│   ├── serial.c             # COM1 serial driver
│   └── net_ne2k.c           # NE2000 NIC skeleton
├── fs/
│   ├── vfs_simple.c         # VFS layer
│   └── ramfs.c              # In-memory filesystem
├── include/                 # System headers
├── Makefile                 # Build system
├── link.ld                  # Linker script (kernel at 0x8000)
└── os.img                   # Generated bootable disk image
```

## Architecture

```
┌─────────────────────────────────────┐
│           Shell (shell.c)           │
├─────────────────────────────────────┤
│  Security │ Monitor │ Power │ ELF   │
├─────────────────────────────────────┤
│ Process │ Memory │ IDT │ Syscalls  │
├─────────────────────────────────────┤
│ VGA │ Keyboard │ Timer │ Serial │ PCI│
├─────────────────────────────────────┤
│     Boot → Protected Mode Entry     │
└─────────────────────────────────────┘
```

## Boot Sequence

1. BIOS loads bootloader from MBR to `0x7C00` (16-bit real mode)
2. Bootloader loads kernel to `0x8000`, sets up GDT, enables protected mode
3. Kernel entry (`_start`) initializes stack, calls `kmain()`
4. `kmain()` initializes all subsystems, enables interrupts, launches shell

## Limitations

This is an educational OS. Notable limitations:

- No `kfree()` — heap allocator is bump-only
- RAMFS is in-memory only (no disk I/O)
- Network stack is a skeleton (no real packet transmission)
- Fixed process table (max 8 processes)
- VGA text mode only (no graphics)

## Acknowledgments

- [OSDev Wiki](https://wiki.osdev.org/) — Essential OS development reference
- [JamesM's Kernel Development Tutorials](http://www.jamesmolloy.co.uk/tutorial_html/)
- [BrokenThorn Entertainment OS Development Series](http://www.brokenthorn.com/Resources/)

## License

Educational use. Contact for commercial licensing.
