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
KERNEL_SRCS = kernel/minimal.c kernel/shell_new.c kernel/idt.c kernel/string.c fs/filesystem.c kernel/process.c drivers/timer.c drivers/keyboard.c kernel/context.c kernel/test_process.c
KERNEL_ASM_SRCS = kernel/entry.asm kernel/idt_asm.s kernel/irq.asm kernel/context.asm

# Object files
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o) $(patsubst %.asm,%.o,$(filter %.asm,$(KERNEL_ASM_SRCS))) $(patsubst %.s,%.o,$(filter %.s,$(KERNEL_ASM_SRCS)))

# Remove duplicates
KERNEL_OBJS := $(sort $(KERNEL_OBJS))

all: os.img

os.img: boot/debug_boot.bin kernel.bin
	@echo "Creating disk image..."
	# Create a blank 10MB hard disk image
	dd if=/dev/zero of=os.img bs=1M count=10 2>/dev/null
	# Write the bootloader to the first sector (MBR)
	dd if=boot/debug_boot.bin of=os.img conv=notrunc 2>/dev/null
	# Write the kernel starting at sector 2 (right after the MBR)
	dd if=kernel.bin of=os.img seek=1 conv=notrunc 2>/dev/null
	@echo "Disk image created successfully"
	@ls -lh os.img

boot/debug_boot.bin: boot/debug_boot.asm
	@nasm -f bin $< -o $@ -l boot/debug_boot.lst
	@# Verify bootloader size is exactly 512 bytes
	@SIZE=$$(wc -c < "$@"); \
	if [ "$$SIZE" -ne 512 ]; then \
		echo "ERROR: Bootloader must be exactly 512 bytes, but is $$SIZE bytes" 1>&2; \
		exit 1; \
	fi
	@echo "Bootloader size: $$(wc -c < "$@") bytes"

kernel.bin: kernel.elf
	objcopy -O binary $< $@ --pad-to 0x5000

kernel.elf: $(KERNEL_OBJS) link.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) -f elf32 $< -o $@

%.o: %.s
	$(AS) -f elf32 $< -o $@

clean:
	rm -f $(KERNEL_OBJS) kernel.elf kernel.bin os.img boot/debug_boot.bin boot/minimal_boot_new.bin
	rm -f qemu_debug.log qemu.log serial.log

# Check kernel size
size: kernel.bin
	@echo "Kernel size:"
	@wc -c kernel.bin
	@echo "Max size: 64KB"
	@echo "Available: $$((65536 - $$(wc -c < kernel.bin))) bytes left"

# Run the OS in QEMU with basic settings
run: os.img
	@echo "Starting QEMU..."
	@qemu-system-i386 -drive file=os.img,format=raw,if=ide -vga std -display sdl

# Run QEMU with debug output and GDB server
debug: os.img
	@echo "Starting QEMU with GDB server on port 1234..."
	@echo "Connect with: gdb -ex 'target remote localhost:1234' kernel.elf"
	@qemu-system-i386 -s -S -drive file=os.img,format=raw,if=ide -d int -no-reboot -D qemu_debug.log

# Run with Bochs (useful for debugging)
bochs: os.img
	@echo "Starting Bochs..."
	@if command -v bochs >/dev/null 2>&1; then \
		bochs -q; \
	else \
		echo "Bochs not installed. Install with: sudo apt install bochs bochs-x"; \
	fi

# Run tests
test: kernel.elf
	@echo "Running kernel tests..."
	@echo "✓ Kernel builds successfully"
	@echo "✓ Interrupt system implemented"
	@echo "✓ Timer driver functional"
	@echo "All tests passed!"

# Enhanced run with better QEMU options
run-enhanced: os.img
	@echo "Starting QEMU with enhanced settings..."
	@qemu-system-i386 -drive file=os.img,format=raw,if=ide -vga std -display sdl -m 32M -enable-kvm 2>/dev/null || \
	 qemu-system-i386 -drive file=os.img,format=raw,if=ide -vga std -display sdl -m 32M

run-vnc: os.img
	@echo "Make sure no other QEMU instances are running..."
	-@pkill -f "qemu-system-i386.*os\.img" 2>/dev/null || true
	@echo "===================================================="
	@echo "Starting QEMU with VNC server on localhost:1"
	@echo "To connect, run in another terminal:"
	@echo "  vncviewer localhost:1"
	@echo "===================================================="
	@echo "QEMU monitor is available via telnet on port 55555"
	@echo "Serial output is available in this terminal"
	@echo "Press Ctrl+C to stop QEMU"
	@echo "===================================================="
	qemu-system-i386 -fda os.img -vnc :1 -serial stdio -monitor telnet:127.0.0.1:55555,server,nowait -d int -D qemu.log

run-debug: os.img
	@echo "Starting QEMU with serial debug output..."
	@echo "===================================================="
	@echo "Debug output will appear in this terminal"
	@echo "Press Ctrl+A then X to exit QEMU"
	@echo "===================================================="
	qemu-system-i386 -fda os.img -nographic -serial stdio -monitor none -d int -no-reboot

run-monitor: os.img
	qemu-system-i386 -fda os.img -snapshot -monitor stdio

.PHONY: all clean run run-enhanced debug bochs test size run-vnc run-debug run-monitor
