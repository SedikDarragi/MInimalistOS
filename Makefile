CC = gcc
AS = nasm
LD = ld

# Cross-compiler for 32-bit
ifneq (,$(findstring i686-elf-,$(shell which i686-elf-gcc 2>/dev/null)))
    CC = i686-elf-gcc
    LD = i686-elf-ld
else
    CFLAGS += -m32
    LDFLAGS += -m elf_i386
endif

# Base compiler flags
INCLUDES = -I./include -I./kernel
CFLAGS += -ffreestanding -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs \
          -Wall -Wextra -Werror $(INCLUDES) -g

# Linker flags
LDFLAGS += -T link.ld -nostdlib

# Assembler flags
ASFLAGS = -f elf32

SOURCES_C = $(wildcard kernel/*.c) $(wildcard drivers/*.c) $(wildcard fs/*.c) $(wildcard net/*.c) $(wildcard ui/*.c) kernel/process.c kernel/utils.c
SOURCES_ASM = $(wildcard kernel/*.asm)
OBJECTS_C = $(SOURCES_C:.c=.o)
OBJECTS_ASM = $(SOURCES_ASM:.asm=.o)

all: os.bin

os.bin: boot/boot.bin kernel.bin
	cat boot/boot.bin kernel.bin > os.bin

kernel.bin: kernel/start.o $(OBJECTS_C)
	$(LD) $(LDFLAGS) kernel/start.o $(OBJECTS_C) -o kernel.elf
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
	qemu-system-i386 -fda os.bin -vnc :1 -k en-us

run-vnc: os.bin
	qemu-system-i386 -fda os.bin -vnc :1 -k en-us

run-debug: os.bin
	qemu-system-i386 -fda os.bin -nographic -serial stdio

run-monitor: os.bin
	qemu-system-i386 -fda os.bin -monitor stdio

.PHONY: all clean run
