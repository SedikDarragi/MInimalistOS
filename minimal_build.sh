#!/bin/bash
cd "/home/slime/Documents/OS copy copy copy"

echo "Minimal build - testing kernel loading..."
echo "Working directory: $(pwd)"
echo "Starting at: $(date)"

# Compile assembly file
echo "Compiling assembly file..."
nasm -f elf32 kernel/minimal_entry.s -o kernel/minimal_entry.o 2>&1 && echo "minimal_entry.o OK" || echo "minimal_entry.o failed"

# Link kernel
echo "Linking minimal kernel..."
ld -m elf_i386 -T link_minimal.ld -nostdlib -z max-page-size=0x1000 -o kernel_minimal.elf kernel/minimal_entry.o 2>&1

if [ $? -eq 0 ]; then
    echo "Minimal kernel linked successfully!"
    ls -la kernel_minimal.elf
    
    # Create binary
    objcopy -O binary kernel_minimal.elf kernel_minimal.bin --pad-to 0x5000
    echo "Minimal kernel binary created: $(stat -c%s kernel_minimal.bin) bytes"
    
    # Create disk image
    if [ -f boot/debug_boot.bin ]; then
        dd if=/dev/zero of=os_minimal.img bs=1M count=10 2>/dev/null
        dd if=boot/debug_boot.bin of=os_minimal.img conv=notrunc 2>/dev/null
        dd if=kernel_minimal.bin of=os_minimal.img seek=1 conv=notrunc 2>/dev/null
        echo "Minimal disk image created: os_minimal.img"
        ls -la os_minimal.img
    else
        echo "Bootloader missing, building it..."
        nasm -f bin boot/debug_boot.asm -o boot/debug_boot.bin
        dd if=/dev/zero of=os_minimal.img bs=1M count=10 2>/dev/null
        dd if=boot/debug_boot.bin of=os_minimal.img conv=notrunc 2>/dev/null
        dd if=kernel_minimal.bin of=os_minimal.img seek=1 conv=notrunc 2>/dev/null
        echo "Minimal disk image created: os_minimal.img"
        ls -la os_minimal.img
    fi
else
    echo "Linking failed"
fi