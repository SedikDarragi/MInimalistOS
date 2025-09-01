#!/bin/bash

# MinimalOS Build Script

echo "Building MinimalOS..."

# Check for required tools
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo "Error: $1 is not installed"
        echo "Please install: sudo apt-get install $2"
        exit 1
    fi
}

echo "Checking dependencies..."
check_tool "gcc" "gcc-multilib"
check_tool "nasm" "nasm"
check_tool "make" "make"
check_tool "qemu-system-i386" "qemu-system-x86"

# Create directories if they don't exist
mkdir -p boot kernel drivers fs net ui

# Build the OS
echo "Building bootloader and kernel..."
make clean
make

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo ""
    echo "To run the OS:"
    echo "  make run"
    echo ""
    echo "Or manually with QEMU:"
    echo "  qemu-system-i386 -fda os.bin"
    echo ""
    echo "Available commands in the OS:"
    echo "  help, clear, echo, ls, cat, mkdir, rm, cd, pwd"
    echo "  ps, kill, ping, ifconfig, ui"
    echo ""
    echo "Press 'ui' in the shell to launch the graphical interface"
else
    echo "Build failed!"
    exit 1
fi
