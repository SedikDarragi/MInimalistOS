CC = gcc
AS = nasm
LD = ld

# Cross-compiler for 32-bit
ifneq (,$(findstring i686-elf-,$(shell which i686-elf-gcc 2>/dev/null)))
    CC = i686-elf-gcc
    AS = i686-elf-as
    LD = i686-elf-ld
    CFLAGS += -ffreestanding -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
              -Wall -Wextra -Werror $(INCLUDES) -g -D__is_kernel -I.\
              -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx
else
    # Try to use system gcc with 32-bit
    CC = gcc
    AS = nasm
    LD = ld
    CFLAGS += -m32 -ffreestanding -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
              -Wall -Wextra -Werror $(INCLUDES) -g -D__is_kernel -I.\
              -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx \
              -I./include -I./kernel \
              -O0 -fno-omit-frame-pointer -fno-pie -fno-pic \
              -fno-common -fno-strict-aliasing -fomit-frame-pointer

    LDFLAGS += -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000
    
    # Check if 32-bit libraries are installed (check for Arch Linux first, then Debian/Ubuntu)
    ifeq (,$(wildcard /usr/lib32/libc.so))
        ifeq (,$(wildcard /usr/lib/i386-linux-gnu/libc.so))
            $(error 32-bit libraries not found! On Arch Linux: sudo pacman -S lib32-gcc-libs lib32-glibc. On Debian/Ubuntu: sudo apt install gcc-multilib)
        endif
    endif
endif

# Base compiler flags
INCLUDES = -I./include -I./kernel -I.

# Linker flags (only set once)
LDFLAGS = -m elf_i386 -T link.ld -nostdlib

# Compiler flags
CFLAGS += -nostdinc -fno-builtin -fno-common -fno-strict-aliasing -fomit-frame-pointer \
          -I./include -I./kernel -I.

# Assembler flags
ASFLAGS = -f elf32

SOURCES_C = $(wildcard kernel/*.c) $(wildcard drivers/*.c) $(wildcard fs/*.c) $(wildcard net/*.c) $(wildcard ui/*.c) kernel/process.c kernel/utils.c
SOURCES_ASM = $(wildcard kernel/*.asm)
OBJECTS_C = $(SOURCES_C:.c=.o)
OBJECTS_ASM = $(SOURCES_ASM:.asm=.o)

KERNEL_OBJS = kernel/start.o kernel/kernel.o kernel/process.o kernel/shell.o kernel/utils.o \
              drivers/keyboard.o drivers/timer.o drivers/vga.o \
              fs/filesystem.o net/network.o ui/ui.o

all: os.bin

os.bin: boot/boot.bin kernel.bin
	cat boot/boot.bin kernel.bin > os.bin

kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o kernel.elf
	objcopy -O binary kernel.elf kernel.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

boot/boot.bin: boot/boot.asm
	$(AS) -f bin $< -o $@

clean:
	rm -f $(OBJECTS_C) $(OBJECTS_ASM) kernel/start.o kernel.elf kernel.bin os.bin boot/boot.bin

run: os.bin
	@echo "Make sure no other QEMU instances are running..."
	-@pkill -f "qemu-system-i386.*os\.bin" 2>/dev/null || true
	@echo "Starting QEMU..."
	qemu-system-i386 -fda os.bin -snapshot -nographic -monitor none -serial stdio

run-vnc: os.bin
	@echo "Make sure no other QEMU instances are running..."
	-@pkill -f "qemu-system-i386.*os\.bin" 2>/dev/null || true
	@echo "Starting QEMU with VNC (connect with vncviewer localhost:1) and serial output..."
	qemu-system-i386 -fda os.bin -snapshot -vnc :1 -k en-us -serial stdio -no-kvm -d int,cpu_reset -D qemu.log

run-debug: os.bin
	qemu-system-i386 -fda os.bin -snapshot -nographic -serial stdio

run-monitor: os.bin
	qemu-system-i386 -fda os.bin -snapshot -monitor stdio

.PHONY: all clean run
