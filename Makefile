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
LDFLAGS = -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000

# Compiler flags
CFLAGS += -m32 -ffreestanding -fno-builtin -fno-stack-protector \
          -Wall -Wextra -Werror -nostdlib -nostartfiles -nodefaultlibs \
          -fno-pie -fno-pic -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx \
          -I./include -I./kernel -I. -g

# Assembler flags
ASFLAGS = -f elf32

# Source files
KERNEL_SRCS = kernel/minimal.c
KERNEL_ASM_SRCS = kernel/entry.asm

# Object files
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o) $(KERNEL_ASM_SRCS:.asm=.o)

all: os.img

os.img: boot/minimal_boot.bin kernel.bin
	dd if=/dev/zero of=os.img bs=512 count=2880
	dd if=boot/minimal_boot.bin of=os.img conv=notrunc
	dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc

boot/minimal_boot.bin: boot/minimal_boot.asm
	nasm -f bin $< -o $@

kernel.bin: kernel.elf
	objcopy -O binary $< $@
	# Ensure the binary is properly aligned
	truncate -s 4K $@

kernel.elf: $(KERNEL_OBJS) link.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) $(ASFLAGS) $< -o $@

boot/boot.bin: boot/boot.asm
	$(AS) -f bin $< -o $@

clean:
	rm -f $(KERNEL_OBJS) kernel.elf kernel.bin os.img boot/boot.bin boot/minimal_boot.bin

run: os.img
	@echo "Make sure no other QEMU instances are running..."
	-@pkill -f "qemu-system-i386.*os\.img" 2>/dev/null || true
	@echo "Starting QEMU..."
	qemu-system-i386 -fda os.img -snapshot -nographic -monitor none -serial stdio

run-vnc: os.img
	@echo "Make sure no other QEMU instances are running..."
	-@pkill -f "qemu-system-i386.*os\.img" 2>/dev/null || true
	@echo "Starting QEMU with VNC (connect with vncviewer localhost:1) and serial output..."
	qemu-system-i386 -fda os.img -snapshot -vnc :1 -k en-us -serial stdio -no-kvm -d int,cpu_reset -D qemu.log

run-debug: os.img
	qemu-system-i386 -fda os.img -snapshot -nographic -serial stdio -d int -D qemu_debug.log

run-monitor: os.img
	qemu-system-i386 -fda os.img -snapshot -monitor stdio

.PHONY: all clean run run-vnc run-debug run-monitor
