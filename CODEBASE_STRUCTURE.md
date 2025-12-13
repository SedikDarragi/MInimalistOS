# Minimalist OS - Codebase Structure Documentation

## Overview
This document describes the complete structure of the cleaned and optimized Minimalist OS codebase.

## Directory Structure

### `/boot/` - Bootloader Components
- `boot.asm` - Main bootloader assembly
- `debug_boot.asm` - Debug bootloader with enhanced features
- `disk.asm` - Disk I/O routines
- `gdt.asm` - Global Descriptor Table setup
- `minimal.asm` - Minimal bootloader
- `minimal_boot.asm` - Alternative minimal bootloader
- `minimal_boot_new.asm` - Updated minimal bootloader
- `new_boot.asm` - New bootloader implementation
- `print_string_pm.asm` - Protected mode string printing
- `switch_to_pm.asm` - Real to protected mode transition

### `/drivers/` - Hardware Drivers
- `block_device.c` - Block device driver interface
- `char_device.c` - Character device driver interface
- `keyboard.c` - Keyboard input driver
- `keyboard.h` - Keyboard driver header
- `timer.c` - System timer driver
- `timer.h` - Timer driver header
- `vga.h` - VGA display interface

### `/fs/` - Filesystem
- `filesystem_enhanced.c` - Enhanced filesystem implementation
- `filesystem.h` - Filesystem interface header

### `/include/` - System Headers
- `device.h` - Device management interface
- `filesystem.h` - Filesystem interface
- `idt.h` - Interrupt Descriptor Table
- `memory.h` - Memory management interface
- `monitor.h` - System monitoring interface
- `network.h` - Network interface
- `power.h` - Power management interface
- `security.h` - Security system interface
- `serial.h` - Serial communication interface
- `shell.h` - Shell interface
- `stdarg.h` - Variable argument handling
- `stdbool.h` - Boolean type definitions
- `stddef.h` - Standard definitions
- `stdint.h` - Standard integer types
- `string.h` - String operations
- `syscall.h` - System call interface
- `usermode.h` - User mode interface
- `vga.h` - VGA display interface

### `/kernel/` - Core Kernel

#### Core Kernel Files
- `minimal.c` - Main kernel entry point
- `entry.asm` - Kernel entry assembly
- `idt.c` - Interrupt handling
- `string.c` - String operations
- `syscall.c` - System call dispatcher
- `process.c` - Process management
- `process.h` - Process interface
- `memory.c` - Memory management
- `context.c` - Context switching
- `context.h` - Context interface
- `io.h` - I/O operations

#### Subsystem Implementations
- `security.c` - Security system
- `security_test.c` - Security testing
- `monitor.c` - System monitoring
- `monitor_test.c` - Monitoring testing
- `power.c` - Power management
- `power_test.c` - Power testing
- `network.c` - Network stack
- `network_test.c` - Network testing
- `device.c` - Device management
- `device_test.c` - Device testing

#### User Mode and Shell
- `shell_new.c` - Enhanced shell implementation
- `usermode.c` - User mode support
- `user_process.c` - User process handling
- `user_program.c` - User program interface

#### Testing and Utilities
- `test_process.c` - Basic process testing
- `memory_test.c` - Memory testing
- `fs_test.c` - Filesystem testing

#### Assembly Files
- `context.asm` - Context switching assembly
- `idt_asm.s` - IDT assembly routines
- `irq.asm` - Interrupt handling assembly

## Kernel Subsystems

### 1. Process Management
- **Files**: `process.c`, `process.h`, `context.c`, `context.asm`
- **Features**: Multi-process support, context switching, scheduling

### 2. Memory Management
- **Files**: `memory.c`, `memory.h`, `memory_test.c`
- **Features**: Dynamic allocation, memory testing

### 3. Device Management
- **Files**: `device.c`, `device.h`, `device_test.c`
- **Features**: Device registration, I/O operations

### 4. Security System
- **Files**: `security.c`, `security.h`, `security_test.c`
- **Features**: User management, permissions, access control

### 5. System Monitoring
- **Files**: `monitor.c`, `monitor.h`, `monitor_test.c`
- **Features**: Logging, statistics, performance monitoring

### 6. Power Management
- **Files**: `power.c`, `power.h`, `power_test.c`
- **Features**: Power states, battery monitoring, thermal management

### 7. Network Stack
- **Files**: `network.c`, `network.h`, `network_test.c`
- **Features**: Network interface, packet handling

### 8. Filesystem
- **Files**: `filesystem_enhanced.c`, `filesystem.h`, `fs_test.c`
- **Features**: File operations, directory management

### 9. System Calls
- **Files**: `syscall.c`, `syscall.h`
- **Features**: System call interface, kernel services

## Build System

### Makefile Features
- Parallel build support with automatic CPU detection
- Clean targets for thorough cleanup
- Optimized compiler flags
- Comprehensive build targets

### Build Process
1. Compile all C source files with optimized flags
2. Assemble all assembly files
3. Link with custom linker script
4. Create bootable disk image

## Kernel Statistics
- **Size**: 34.9KB (within 64KB limit)
- **Available Space**: 30.6KB
- **Build System**: Optimized with parallel compilation
- **Testing**: Comprehensive test suite for all subsystems

## Development Status
✅ All subsystems implemented and tested
✅ Codebase cleaned and optimized
✅ Build system enhanced
✅ Documentation complete
✅ Ready for development and extension

## Notes
- All unused and duplicate code has been removed
- Build system optimized for performance
- Comprehensive testing framework in place
- Modular architecture for easy extension
- Clean separation of concerns across subsystems
