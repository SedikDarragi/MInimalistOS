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
KERNEL_SRCS = kernel/minimal.c kernel/shell_new.c kernel/idt.c
KERNEL_ASM_SRCS = kernel/entry.asm kernel/idt_asm.s kernel/irq.asm

# Object files
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o) $(patsubst %.asm,%.o,$(filter %.asm,$(KERNEL_ASM_SRCS))) $(patsubst %.s,%.o,$(filter %.s,$(KERNEL_ASM_SRCS)))

all: os.img

os.img: boot/minimal_boot_new.bin kernel.bin
	echo "Creating disk image..."
	dd if=/dev/zero of=os.img bs=512 count=2880 status=none
	echo "Writing bootloader..."
	dd if=boot/minimal_boot_new.bin of=os.img conv=notrunc status=none
	echo "Writing kernel..."
	dd if=kernel.bin of=os.img bs=512 seek=1 conv=notrunc status=none
	# Ensure boot signature is present
	echo -n -e '\x55\xAA' | dd of=os.img bs=1 seek=510 conv=notrunc status=none
	echo "Verifying boot signature..."
	@if ! dd if=os.img bs=1 skip=510 count=2 2>/dev/null | hexdump -v -e '1/1 "%02x"' | grep -q '55aa'; then \
		echo "ERROR: Boot signature missing or incorrect!" 1>&2; \
		hexdump -C -s 510 -n 2 os.img 1>&2; \
		exit 1; \
	fi
	echo "Disk image created successfully"
	@ls -lh os.img

boot/minimal_boot_new.bin: boot/minimal_boot_new.asm
	nasm -f bin $< -o $@
	@# Verify bootloader size
	@SIZE=$$(wc -c < "$@"); \
	if [ "$$SIZE" -gt 510 ]; then \
		echo "ERROR: Bootloader too large ($$SIZE bytes, max 510)" 1>&2; \
		hexdump -C -n 512 "$@" 1>&2; \
		exit 1; \
	fi
	@# Ensure boot signature is not present yet
	@if dd if="$@" bs=1 skip=510 count=2 2>/dev/null | grep -q 'U'; then \
		echo "ERROR: Boot signature already present in bootloader!" 1>&2; \
		hexdump -C -s 500 -n 20 "$@" 1>&2; \
		exit 1; \
	fi
	@echo "Bootloader size: $$(wc -c < "$@") bytes"

kernel.bin: kernel.elf
	objcopy -O binary $< $@
	# Ensure the binary is properly aligned
	truncate -s 4K $@

kernel.elf: $(KERNEL_OBJS) link.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	$(AS) -f elf32 $< -o $@

%.o: %.s
	$(AS) -f elf32 $< -o $@

boot/boot.bin: boot/boot.asm
	$(AS) -f bin $< -o $@

clean:
	rm -f $(KERNEL_OBJS) kernel.elf kernel.bin os.img boot/boot.bin boot/minimal_boot.bin boot/minimal_boot_new.bin
	rm -f qemu_debug.log

run: os.img
	@echo "Make sure no other QEMU instances are running..."
	-@pkill -f "qemu-system-i386.*os\.img" 2>/dev/null || true
	@echo "Starting QEMU..."
	qemu-system-i386 -fda os.img -snapshot -nographic -monitor none -serial stdio -d int -no-reboot

run-vnc: os.img
	@echo "Make sure no other QEMU instances are running..."
	-@pkill -f "qemu-system-i386.*os\.img" 2>/dev/null || true
	@echo "Starting QEMU with VNC (connect with vncviewer localhost:1) and serial output..."
	qemu-system-i386 -fda os.img -snapshot -vnc :1 -k en-us -serial stdio -no-kvm -d int,cpu_reset -D qemu.log

run-debug: os.img
	@echo "Starting QEMU with debug output..."
	qemu-system-i386 -fda os.img -snapshot -nographic -d int -no-reboot

run-monitor: os.img
	qemu-system-i386 -fda os.img -snapshot -monitor stdio

.PHONY: all clean run run-vnc run-debug run-monitor
