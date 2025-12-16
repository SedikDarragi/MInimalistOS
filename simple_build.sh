#!/bin/bash

# Simple build script for the OS kernel

echo "Building OS kernel..."

# Clean up old files
rm -f *.o kernel.elf kernel.bin os.img
find . -name "*.o" -delete 2>/dev/null || true

# Compile C files
echo "Compiling C files..."
gcc -m32 -c -ffreestanding -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -g -D__is_kernel -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fno-omit-frame-pointer -fno-pie -fno-pic -fno-common -fno-strict-aliasing -fomit-frame-pointer -Wall -Wextra -Werror -I./include -I./kernel -I. -I./drivers -I./fs kernel/*.c drivers/*.c fs/*.c

# Compile assembly files
echo "Compiling assembly files..."
nasm -f elf32 kernel/entry.s -o kernel/entry.o
nasm -f elf32 kernel/idt_asm.asm -o kernel/idt_asm.o
nasm -f elf32 kernel/context.asm -o kernel/context_asm.o

# Link the kernel
echo "Linking kernel..."
ld -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000 -o kernel.elf *.o kernel/*.o

# Create binary
echo "Creating binary..."
objcopy -O binary kernel.elf kernel.bin

# Create disk image
echo "Creating disk image..."
dd if=boot/minimal_boot_new.bin of=os.img bs=512 count=1 conv=notrunc 2>/dev/null
dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc 2>/dev/null

echo "Build complete!"
echo "Kernel size: $(stat -c%s kernel.bin) bytes"
