#!/bin/bash
set -e
cd "/home/slime/Documents/OS copy copy copy"

echo "=== Full Build Script ==="
echo "Working directory: $(pwd)"

if [ "$1" == "clean" ]; then
    echo "Cleaning..."
    rm -f *.o kernel/*.o drivers/*.o fs/*.o kernel.elf kernel.bin os.img 2>/dev/null || true
    echo "Clean complete"
    exit 0
fi

# Check tools
echo "Checking tools..."
which gcc && echo "gcc: OK" || echo "gcc: MISSING"
which nasm && echo "nasm: OK" || echo "nasm: MISSING"
which ld && echo "ld: OK" || echo "ld: MISSING"

# Clean
echo "Cleaning..."
rm -f *.o kernel/*.o drivers/*.o fs/*.o kernel.elf kernel.bin os.img 2>/dev/null || true

# Compile log.c first (the file we fixed)
echo "Compiling log.c..."
gcc -m32 -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-stack-protector \
    -nostartfiles -nodefaultlibs -g -D__is_kernel -mno-sse -mno-mmx -mno-sse2 \
    -mno-3dnow -mno-avx -fno-omit-frame-pointer -fno-pic -fno-common \
    -fno-strict-aliasing -fomit-frame-pointer -O0 -g3 \
    -I./include -I./kernel -I. -I./drivers -I./fs \
    -c kernel/log.c -o kernel/log.o 2>&1

if [ $? -eq 0 ]; then
    echo "log.c compiled successfully"
    ls -la kernel/log.o
else
    echo "log.c compilation failed"
    exit 1
fi

# Compile net_ne2k.c (the file that was causing errors)
echo "Compiling net_ne2k.c..."
gcc -m32 -Wall -Wextra -Werror -ffreestanding -fno-builtin -fno-stack-protector \
    -nostartfiles -nodefaultlibs -g -D__is_kernel -mno-sse -mno-mmx -mno-sse2 \
    -mno-3dnow -mno-avx -fno-omit-frame-pointer -fno-pic -fno-common \
    -fno-strict-aliasing -fomit-frame-pointer -O0 -g3 \
    -I./include -I./kernel -I. -I./drivers -I./fs \
    -c drivers/net_ne2k.c -o drivers/net_ne2k.o 2>&1

if [ $? -eq 0 ]; then
    echo "net_ne2k.c compiled successfully"
    ls -la drivers/net_ne2k.o
else
    echo "net_ne2k.c compilation failed"
    exit 1
fi

echo "Build test completed successfully!"