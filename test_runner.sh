#!/bin/bash

echo "=== Minimalist OS Test Runner ==="
echo

# Test 1: Check if main kernel builds
echo "Test 1: Building main kernel..."
if make clean > /dev/null 2>&1 && ./simple_build.sh > /dev/null 2>&1; then
    echo "✓ Main kernel builds successfully"
else
    echo "✗ Main kernel build failed"
    exit 1
fi

# Test 2: Check kernel size
echo "Test 2: Checking kernel size..."
if [ -f kernel.bin ]; then
    SIZE=$(stat -c%s kernel.bin 2>/dev/null || echo 0)
    if [ $SIZE -lt 65536 ]; then
        echo "✓ Kernel size is $SIZE bytes (under 64KB limit)"
    else
        echo "✗ Kernel size is $SIZE bytes (exceeds 64KB limit)"
        exit 1
    fi
else
    echo "✗ Kernel binary not found"
    exit 1
fi

# Test 3: Check disk image creation
echo "Test 3: Checking disk image..."
if [ -f os.img ]; then
    echo "✓ Disk image created successfully"
else
    echo "✗ Disk image not found"
    exit 1
fi

# Test 4: Check essential files exist
echo "Test 4: Checking essential files..."
FILES=("include/string.h" "drivers/vga.h" "kernel/isr.c" "kernel/idt.c")
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file exists"
    else
        echo "✗ $file missing"
        exit 1
    fi
done

# Test 5: Check string functions
echo "Test 5: Checking string function implementations..."
if grep -q "char \*strcpy" kernel/string.c; then
    echo "✓ strcpy function implemented"
else
    echo "✗ strcpy function missing"
    exit 1
fi

if grep -q "void \*memset" kernel/string.c; then
    echo "✓ memset function implemented"
else
    echo "✗ memset function missing"
    exit 1
fi

if grep -q "int strcmp" kernel/string.c; then
    echo "✓ strcmp function implemented"
else
    echo "✗ strcmp function missing"
    exit 1
fi

# Test 6: Check VGA functions
echo "Test 6: Checking VGA driver..."
if grep -q "void vga_print" drivers/vga.c; then
    echo "✓ vga_print function implemented"
else
    echo "✗ vga_print function missing"
    exit 1
fi

if grep -q "void vga_clear" drivers/vga.c; then
    echo "✓ vga_clear function implemented"
else
    echo "✗ vga_clear function missing"
    exit 1
fi

# Test 7: Check ISR functions
echo "Test 7: Checking interrupt handlers..."
if grep -q "void isr0" kernel/isr.c; then
    echo "✓ ISR handlers implemented"
else
    echo "✗ ISR handlers missing"
    exit 1
fi

if grep -q "void irq0" kernel/isr.c; then
    echo "✓ IRQ handlers implemented"
else
    echo "✗ IRQ handlers missing"
    exit 1
fi

echo
echo "=== All Tests Passed! ==="
echo "The Minimalist OS kernel is ready for development."
echo
echo "To run the OS:"
echo "  ./simple_build.sh"
echo "  qemu-system-i386 -m 32M -drive file=os.img,format=raw,if=ide -vga std -display sdl"
