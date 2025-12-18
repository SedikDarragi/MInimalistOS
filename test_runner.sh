#!/bin/bash

echo "=== Comprehensive OS Kernel Test Suite ==="
echo

# Test 1: Check if main kernel components can be built
echo "Test 1: Testing kernel component builds..."
if make clean > /dev/null 2>&1; then
    echo "✓ Build system cleaned successfully"
    
    # Test individual component builds
    if make kernel/string.o kernel/memory.o kernel/log.o drivers/vga.o > /dev/null 2>&1; then
        echo "✓ Core kernel components build successfully"
    else
        echo "✗ Core kernel components build failed"
        exit 1
    fi
else
    echo "✗ Build system cleanup failed"
    exit 1
fi

# Test 2: Check essential files exist
echo "Test 2: Checking essential files..."
FILES=("include/string.h" "drivers/vga.h" "kernel/isr.c" "kernel/idt.c" "drivers/keyboard.h")
for file in "${FILES[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file exists"
    else
        echo "✗ $file missing"
        exit 1
    fi
done

# Test 3: Check string functions
echo "Test 3: Checking string function implementations..."
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

# Test 4: Check VGA functions
echo "Test 4: Checking VGA driver..."
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

# Test 5: Check ISR functions
echo "Test 5: Checking interrupt handlers..."
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

# Test 6: Build and test memory management
echo "Test 6: Testing memory management..."
if make clean > /dev/null 2>&1 && make memory-test > /dev/null 2>&1; then
    echo "✓ Memory test kernel builds successfully"
    if [ -f memory_test.img ]; then
        echo "✓ Memory test disk image created"
    else
        echo "✗ Memory test disk image missing"
        exit 1
    fi
else
    echo "✗ Memory test kernel build failed"
    exit 1
fi

# Test 7: Build and test interrupt handling
echo "Test 7: Testing interrupt handling..."
if make clean > /dev/null 2>&1 && make interrupt-test > /dev/null 2>&1; then
    echo "✓ Interrupt test kernel builds successfully"
    if [ -f interrupt_test.img ]; then
        echo "✓ Interrupt test disk image created"
    else
        echo "✗ Interrupt test disk image missing"
        exit 1
    fi
else
    echo "✗ Interrupt test kernel build failed"
    exit 1
fi

# Test 8: Build and test keyboard driver
echo "Test 8: Testing keyboard driver..."
if make clean > /dev/null 2>&1 && make keyboard-test > /dev/null 2>&1; then
    echo "✓ Keyboard test kernel builds successfully"
    if [ -f keyboard_test.img ]; then
        echo "✓ Keyboard test disk image created"
    else
        echo "✗ Keyboard test disk image missing"
        exit 1
    fi
else
    echo "✗ Keyboard test kernel build failed"
    exit 1
fi

# Test 9: Check keyboard functions
echo "Test 9: Checking keyboard driver functions..."
if grep -q "void keyboard_init" drivers/keyboard.c; then
    echo "✓ keyboard_init function implemented"
else
    echo "✗ keyboard_init function missing"
    exit 1
fi

if grep -q "char keyboard_getchar" drivers/keyboard.c; then
    echo "✓ keyboard_getchar function implemented"
else
    echo "✗ keyboard_getchar function missing"
    exit 1
fi

echo
echo "=== All Tests Passed! ==="
echo "The Minimalist OS kernel is ready for development."
echo
echo "Available test commands:"
echo "  make run-simple-test     - Basic kernel functionality test"
echo "  make run-memory-test     - Memory management tests"
echo "  make run-interrupt-test  - Interrupt handling tests"
echo "  make run-keyboard-test   - Keyboard driver tests"
echo
echo "To run the main OS:"
echo "  ./simple_build.sh"
echo "  qemu-system-i386 -m 32M -drive file=os.img,format=raw,if=ide -vga std -display sdl"
