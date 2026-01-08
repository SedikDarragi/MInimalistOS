#!/bin/bash
cd "/home/slime/Documents/OS copy copy copy"

echo "=== Final Build Steps ==="

# Check if we have the object files
echo "Checking for object files..."
OBJ_COUNT=$(find . -name '*.o' -type f | wc -l)
echo "Found $OBJ_COUNT object files"

# Try to link the kernel
echo "Linking kernel..."
ld -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000 -o kernel.elf *.o kernel/*.o 2>&1
if [ $? -eq 0 ]; then
    echo "Kernel linked successfully"
    ls -la kernel.elf
else
    echo "Linking failed, trying alternative..."
    # Try with explicit object files
    ld -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000 -o kernel.elf \
        kernel/kmain.o kernel/log.o kernel/string.o kernel/memory.o \
        kernel/idt.o kernel/isr.o kernel/irq.asm.o \
        drivers/vga.o drivers/keyboard.o drivers/serial.o drivers/timer.o \
        2>&1 || echo "Alternative linking also failed"
fi

# Create binary
if [ -f kernel.elf ]; then
    echo "Creating binary..."
    objcopy -O binary kernel.elf kernel.bin --pad-to 0x5000
    ls -la kernel.bin
fi

# Create disk image
if [ -f kernel.bin ] && [ -f boot/debug_boot.bin ]; then
    echo "Creating disk image..."
    dd if=/dev/zero of=os.img bs=1M count=10 2>/dev/null
    dd if=boot/debug_boot.bin of=os.img conv=notrunc 2>/dev/null
    dd if=kernel.bin of=os.img seek=1 conv=notrunc 2>/dev/null
    ls -la os.img
    echo "Build complete!"
else
    echo "Missing files for disk image"
fi