#!/bin/bash

# Simple build script for the OS kernel

echo "Building OS kernel..."

# Clean up old files
rm -f *.o kernel.elf kernel.bin os.img
find . -name "*.o" -delete 2>/dev/null || true

# Common flags
CFLAGS="-m32 -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -g -D__is_kernel -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fno-omit-frame-pointer -fno-pie -fno-pic -fno-common -fno-strict-aliasing -fomit-frame-pointer -O0 -g3"
INCLUDES="-I./include -I./kernel -I. -I./drivers -I./fs"

# Compile essential kernel files
echo "Compiling kernel files..."
gcc $CFLAGS $INCLUDES -c kernel/kmain.c -o kernel/kmain.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/log.c -o kernel/log.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/string.c -o kernel/string.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/memory.c -o kernel/memory.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/idt.c -o kernel/idt.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/isr.c -o kernel/isr.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/pci.c -o kernel/pci.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/net_core.c -o kernel/net_core.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/network.c -o kernel/network.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/process_simple.c -o kernel/process_simple.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/fs_test.c -o kernel/fs_test.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/syscall_simple.c -o kernel/syscall_simple.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/program_loader.c -o kernel/program_loader.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/context.c -o kernel/context.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/monitor.c -o kernel/monitor.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/power.c -o kernel/power.o 2>&1
gcc $CFLAGS $INCLUDES -c kernel/device.c -o kernel/device.o 2>&1

# Compile essential driver files
echo "Compiling driver files..."
gcc $CFLAGS $INCLUDES -c drivers/vga.c -o drivers/vga.o 2>&1
gcc $CFLAGS $INCLUDES -c drivers/keyboard.c -o drivers/keyboard.o 2>&1
gcc $CFLAGS $INCLUDES -c drivers/keyboard_intl.c -o drivers/keyboard_intl.o 2>&1
gcc $CFLAGS $INCLUDES -c drivers/serial.c -o drivers/serial.o 2>&1
gcc $CFLAGS $INCLUDES -c drivers/timer.c -o drivers/timer.o 2>&1
gcc $CFLAGS $INCLUDES -c drivers/mouse.c -o drivers/mouse.o 2>&1
gcc $CFLAGS $INCLUDES -c drivers/net_ne2k.c -o drivers/net_ne2k.o 2>&1

# Compile filesystem files
echo "Compiling filesystem files..."
gcc $CFLAGS $INCLUDES -c fs/filesystem_enhanced.c -o fs/filesystem_enhanced.o 2>&1

# Compile assembly files
echo "Compiling assembly files..."
gcc -m32 -c kernel/entry.s -o kernel/entry.o 2>&1
nasm -f elf32 kernel/idt_asm.asm -o kernel/idt_asm.o 2>&1
gcc -m32 -c kernel/context.s -o kernel/context.o 2>&1

# Link kernel
echo "Linking kernel..."
ld -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000 -o kernel.elf \
    kernel/entry.o kernel/kmain.o kernel/log.o kernel/string.o kernel/memory.o \
    kernel/idt.o kernel/idt_asm.o kernel/isr.o kernel/pci.o kernel/net_core.o kernel/network.o \
    kernel/process_simple.o kernel/fs_test.o kernel/syscall_simple.o kernel/program_loader.o \
    kernel/context.o kernel/monitor.o kernel/power.o kernel/device.o \
    drivers/vga.o drivers/keyboard.o drivers/keyboard_intl.o drivers/serial.o \
    drivers/timer.o drivers/mouse.o drivers/net_ne2k.o \
    fs/filesystem_enhanced.o 2>&1

if [ $? -eq 0 ]; then
    echo "Kernel linked successfully!"
    ls -la kernel.elf
    
    # Create binary
    objcopy -O binary kernel.elf kernel.bin --pad-to 0x5000
    echo "Kernel binary created: $(stat -c%s kernel.bin) bytes"
    
    # Create disk image
    if [ -f boot/debug_boot.bin ]; then
        dd if=/dev/zero of=os.img bs=1M count=10 2>/dev/null
        dd if=boot/debug_boot.bin of=os.img conv=notrunc 2>/dev/null
        dd if=kernel.bin of=os.img seek=1 conv=notrunc 2>/dev/null
        echo "Disk image created: os.img"
        ls -la os.img
    else
        echo "Bootloader missing, building it..."
        nasm -f bin boot/debug_boot.asm -o boot/debug_boot.bin
        dd if=/dev/zero of=os.img bs=1M count=10 2>/dev/null
        dd if=boot/debug_boot.bin of=os.img conv=notrunc 2>/dev/null
        dd if=kernel.bin of=os.img seek=1 conv=notrunc 2>/dev/null
        echo "Disk image created: os.img"
        ls -la os.img
    fi
else
    echo "Linking failed"
    exit 1
fi

echo "Build complete!"
