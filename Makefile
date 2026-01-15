# Toolchain configuration
HOST_OS := $(shell uname -s)

# Detect if we're cross-compiling or using native tools
ifneq (,$(findstring i686-elf-,$(shell which i686-elf-gcc 2>/dev/null)))
    # Cross-compiler toolchain
    TOOLCHAIN_PREFIX = i686-elf-
    CROSS_COMPILING = 1
else
    # Native toolchain with 32-bit support
    TOOLCHAIN_PREFIX =
    CROSS_COMPILING = 0
    
    # Check for 32-bit support on host system
    ifeq (,$(wildcard /usr/lib32/libc.so))
        ifeq (,$(wildcard /usr/lib/i386-linux-gnu/libc.so))
            $(error 32-bit libraries not found! On Arch Linux: sudo pacman -S lib32-gcc-libs lib32-glibc. On Debian/Ubuntu: sudo apt install gcc-multilib)
        endif
    endif
endif

# Tool definitions with prefix
CC = $(TOOLCHAIN_PREFIX)gcc
AS = $(TOOLCHAIN_PREFIX)as
LD = $(TOOLCHAIN_PREFIX)ld
OBJCOPY = $(TOOLCHAIN_PREFIX)objcopy

# Base CFLAGS
BASE_CFLAGS = -Wall -Wextra -Werror \
              -ffreestanding -fno-builtin -fno-stack-protector \
              -nostartfiles -nodefaultlibs \
              -g -D__is_kernel \
              -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx \
              -fno-omit-frame-pointer -fno-pie -fno-pic \
              -fno-common -fno-strict-aliasing -fomit-frame-pointer

# Architecture-specific flags
ifeq ($(CROSS_COMPILING),0)
    CFLAGS += -m32
endif

# Optimizations (enable with make OPTIMIZE=1)
ifdef OPTIMIZE
    CFLAGS += -O2 -fno-strict-aliasing -fomit-frame-pointer
else
    CFLAGS += -O0 -g3
endif

# Warning flags
WBASE_CFLAGS = -Wall -Wextra -Werror \
              -ffreestanding -fno-stack-protector -fno-builtin \
              -fno-exceptions -fno-rtti -fno-common \
              -Wno-unused-parameter -Wno-unused-function \
              -Wnested-externs -Winline -Wno-long-long \
              -Wuninitialized -Wstrict-prototypes

# Base flags
# Include directories
INCLUDES = -I./include -I./kernel -I. -I./drivers -I./fs
LDFLAGS = -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000
ASFLAGS = -f elf32

# Source file organization
KERNEL_SRCS := $(shell find kernel/ -name '*.c' -not -name 'test_*.c' -not -name '*_test.c' -not -name 'tests.c' -not -name 'string_tests.c' -not -name 'process.c' -not -name 'monitor.c' -not -name 'power.c' -not -name 'interrupt_test.c' -not -name 'minimal.c' -not -name 'usermode.c' -not -name 'shell_new.c' -not -name 'fs_test.c' -not -name 'device_test.c' -not -name 'monitor_test.c' -not -name 'network_test.c' -not -name 'memory_test.c' -not -name 'syscall.c' -not -name 'syscall_wrap.c') kernel/kmain.c kernel/process_simple.c kernel/syscall_simple.c kernel/program_loader.c kernel/net_core.c kernel/pci.c
KERNEL_TEST_SRCS := $(shell find kernel/ -name '*_test.c')
TEST_SRCS := kernel/tests.c
DRIVER_SRCS := $(shell find drivers/ -name '*.c')
FS_SRCS := $(shell find fs/ -name '*.c' -not -name 'vfs.c' -not -name 'ramfs.c')

# Assembly sources (both .s and .asm) 
KERNEL_ASM_SRCS := $(shell find kernel/ -name '*.s' -o -name '*.asm')

# Combine all source files
# Main kernel should NOT include test sources; keep tests only in TEST_ALL_SRCS
ALL_SRCS := $(KERNEL_SRCS) $(DRIVER_SRCS) $(FS_SRCS)
TEST_ALL_SRCS := $(KERNEL_SRCS) $(TEST_SRCS) $(KERNEL_TEST_SRCS) $(DRIVER_SRCS) $(FS_SRCS)

# Generate dependencies
DEPS := $(ALL_SRCS:.c=.d) $(patsubst %.s,%.d,$(filter %.s,$(KERNEL_ASM_SRCS)))

# Object files
KERNEL_OBJS = $(ALL_SRCS:.c=.o) \
              $(patsubst %.s,%.o,$(filter %.s,$(KERNEL_ASM_SRCS)))

# Remove duplicates and sort
KERNEL_OBJS := $(sort $(KERNEL_OBJS))

# Include generated dependencies
-include $(DEPS)

# Build configuration
BUILD_DIR = build
BUILD_TYPE ?= debug

# Set optimization level based on build type
ifeq ($(BUILD_TYPE),release)
    CFLAGS += -O2 -DNDEBUG
else
    CFLAGS += -O0 -g3 -DDEBUG
endif

# Enable parallel builds with auto-detection of CPU cores
NUM_CPUS := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
MAKEFLAGS += -j$(NUM_CPUS)

# Verbose build (enable with make V=1)
ifdef V
    Q =
    E = @:
else
    Q = @
    E = @echo
endif

# Optimized build targets
.PHONY: all clean run run-enhanced debug bochs test size run-vnc run-debug run-monitor clean-all

all: os.img

# Clean targets
clean:
	$(E) "  CLEAN"
	@rm -f $(KERNEL_OBJS) kernel.elf kernel.bin os.img
	@rm -f boot/*.bin boot/*.lst
	@rm -f qemu_*.log serial.log

# Deep clean
clean-all: clean
	$(E) "  CLEAN   All generated files"
	@find . -name "*.o" -delete 2>/dev/null || true
	@find . -name "*.d" -delete 2>/dev/null || true
	@find . -name "*.bin" -delete 2>/dev/null || true
	@find . -name "*.elf" -delete 2>/dev/null || true
	@find . -name "*.log" -delete 2>/dev/null || true
	@rm -rf $(BUILD_DIR)

# Create dependency files
%.d: %.c
	@$(CC) $(CFLAGS) -MM -MT "$*.o $@" -o $@ $<

# Run comprehensive test suite
test-all:
	@echo "Running comprehensive OS kernel test suite..."
	@chmod +x test_runner.sh
	@./test_runner.sh

# Phony targets
.PHONY: all clean clean-all run debug test size run-test simple-test run-simple-test memory-test run-memory-test interrupt-test run-interrupt-test keyboard-test run-keyboard-test test-all

os.img: boot/debug_boot.bin kernel.bin
	@echo "Creating disk image..."
	# Create a blank 10MB hard disk image
	dd if=/dev/zero of=os.img bs=1M count=10 2>/dev/null
	# Write the bootloader to the first sector (MBR)
	dd if=boot/debug_boot.bin of=os.img conv=notrunc 2>/dev/null
	# Write the kernel starting at sector 4 (0x2000 = 4 sectors)
	dd if=kernel.bin of=os.img seek=4 conv=notrunc 2>/dev/null
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

# Linker flags
LDFLAGS += -Map=$(BUILD_DIR)/kernel.map --gc-sections

# Link kernel
kernel.elf: $(KERNEL_OBJS) link.ld | $(BUILD_DIR)
	$(E) "  LD      $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)
	$(E) "  SIZE    $@"
	@$(SIZE) $@ || true

# Link test kernel (minimal version)
tests.elf: kernel/test.o kernel/string_tests.o kernel/tests.o drivers/vga.o kernel/string.o | $(BUILD_DIR)
	$(E) "  LD      $@"
	$(Q)$(LD) -m elf_i386 -T link_test.ld -nostdlib -z max-page-size=0x1000 -o $@ $^
	$(E) "  SIZE    $@"
	@$(SIZE) $@ || true

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Pattern rule for compiling C files
%.o: %.c
	$(E) "  CC      $@"
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

# Pattern rule for GAS assembly files (.s)
%.o: %.s
	$(E) "  AS      $@"
	$(Q)$(CC) -m32 -c $< -o $@

# Pattern rule for NASM assembly files (.asm)
%.o: %.asm
	$(E) "  NASM    $@"
	$(Q)nasm -f elf32 $< -o $@


# Size information
size: kernel.elf
	@echo "\n=== Kernel Size Information ==="
	@$(SIZE) kernel.elf || true
	@echo "\nSection sizes:"
	@$(OBJDUMP) -h kernel.elf | \
	    awk '/^\s*[0-9]+\s+\S+\s+[0-9a-f]+/ {printf "%-20s %8s bytes\n", $$2, $$3}' || true
	@echo "\nTotal size: $$(stat -c%s kernel.bin) bytes"
	@echo "Available: $$((65536 - $$(stat -c%s kernel.bin))) bytes left (64KB total)"

# QEMU configuration
QEMU = qemu-system-i386

# Attach a NE2000 PCI NIC so the skeleton NE2K driver has matching hardware
QEMU_NET = -netdev user,id=n0 -device ne2k_pci,netdev=n0

QEMU_OPTS = -m 32M -monitor stdio -no-reboot -no-shutdown $(QEMU_NET)
QEMU_DISK = -drive file=os.img,format=raw,if=ide
QEMU_DEBUG = -d int,cpu_reset -D qemu.log

# Run the OS in QEMU with basic settings
run: os.img
	$(E) "  QEMU    $<"
	@$(QEMU) $(QEMU_OPTS) $(QEMU_DISK) -vga std -display sdl

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
test: tests.elf
	@echo "Running kernel tests..."
	@echo "✓ Test kernel built successfully"
	@echo "✓ Test framework implemented"
	@echo "✓ String tests included"
	@echo "Run 'make run-test' to execute tests in QEMU"

# Simple test kernel (working version)
simple-test: kernel/simple_test.o kernel/string.o kernel/log.o drivers/vga.o
	@echo "Building simple test kernel..."
	$(Q)$(LD) -m elf_i386 -T link_simple_test.ld -nostdlib -z max-page-size=0x1000 -o simple_test.elf $^
	@echo "Simple test kernel built successfully"

# Compile simple test without stack protection
kernel/simple_test.o: kernel/simple_test.c
	$(E) "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -fno-stack-protector $(INCLUDES) -MMD -MP -c $< -o $@

# Run simple test kernel in QEMU
run-simple-test: simple-test
	@echo "Creating test disk image..."
	@objcopy -O binary simple_test.elf simple_test.bin
	@dd if=boot/debug_boot.bin of=simple_test.img bs=512 count=1 conv=notrunc 2>/dev/null
	@dd if=simple_test.bin of=simple_test.img seek=1 conv=notrunc 2>/dev/null
	@echo "Starting QEMU with simple test kernel..."
	@qemu-system-i386 -m 32M -drive file=simple_test.img,format=raw,if=ide -vga std -display sdl -no-reboot

# Memory test kernel
memory-test: kernel/memory_test.o kernel/memory.o kernel/log.o drivers/vga.o kernel/string.o
	@echo "Building memory test kernel..."
	$(Q)$(LD) -m elf_i386 -T link_simple_test.ld -nostdlib -z max-page-size=0x1000 -o memory_test.elf $^
	@echo "Memory test kernel built successfully"

# Compile memory test without stack protection
kernel/memory_test.o: kernel/memory_test.c
	$(E) "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -fno-stack-protector $(INCLUDES) -MMD -MP -c $< -o $@

# Interrupt test kernel
interrupt-test: kernel/interrupt_test.o kernel/idt.o kernel/isr.o kernel/memory.o kernel/log.o drivers/vga.o kernel/string.o
	@echo "Building interrupt test kernel..."
	$(Q)$(LD) -m elf_i386 -T link_simple_test.ld -nostdlib -z max-page-size=0x1000 -o interrupt_test.elf $^
	@echo "Interrupt test kernel built successfully"

# Compile interrupt test without stack protection
kernel/interrupt_test.o: kernel/interrupt_test.c
	$(E) "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -fno-stack-protector $(INCLUDES) -MMD -MP -c $< -o $@

# Keyboard test kernel
keyboard-test: kernel/keyboard_test.o drivers/keyboard.o drivers/vga.o kernel/log.o kernel/string.o
	@echo "Building keyboard test kernel..."
	$(Q)$(LD) -m elf_i386 -T link_simple_test.ld -nostdlib -z max-page-size=0x1000 -o keyboard_test.elf $^
	@echo "Keyboard test kernel built successfully"

# Compile keyboard test without stack protection
kernel/keyboard_test.o: kernel/keyboard_test.c
	$(E) "  CC      $@"
	$(Q)$(CC) $(CFLAGS) -fno-stack-protector $(INCLUDES) -MMD -MP -c $< -o $@

# Run keyboard test kernel in QEMU
run-keyboard-test: keyboard-test
	@echo "Creating keyboard test disk image..."
	@objcopy -O binary keyboard_test.elf keyboard_test.bin
	@dd if=boot/debug_boot.bin of=keyboard_test.img bs=512 count=1 conv=notrunc 2>/dev/null
	@dd if=keyboard_test.bin of=keyboard_test.img seek=1 conv=notrunc 2>/dev/null
	@echo "Starting QEMU with keyboard test kernel..."
	@qemu-system-i386 -m 32M -drive file=keyboard_test.img,format=raw,if=ide -vga std -display sdl -no-reboot

# Run interrupt test kernel in QEMU
run-interrupt-test: interrupt-test
	@echo "Creating interrupt test disk image..."
	@objcopy -O binary interrupt_test.elf interrupt_test.bin
	@dd if=boot/debug_boot.bin of=interrupt_test.img bs=512 count=1 conv=notrunc 2>/dev/null
	@dd if=interrupt_test.bin of=interrupt_test.img seek=1 conv=notrunc 2>/dev/null
	@echo "Starting QEMU with interrupt test kernel..."
	@qemu-system-i386 -m 32M -drive file=interrupt_test.img,format=raw,if=ide -vga std -display sdl -no-reboot

# Run memory test kernel in QEMU
run-memory-test: memory-test
	@echo "Creating memory test disk image..."
	@objcopy -O binary memory_test.elf memory_test.bin
	@dd if=boot/debug_boot.bin of=memory_test.img bs=512 count=1 conv=notrunc 2>/dev/null
	@dd if=memory_test.bin of=memory_test.img seek=1 conv=notrunc 2>/dev/null
	@echo "Starting QEMU with memory test kernel..."
	@qemu-system-i386 -m 32M -drive file=memory_test.img,format=raw,if=ide -vga std -display sdl -no-reboot

# Run test kernel in QEMU (original complex version)
run-test: tests.elf
	@echo "Creating test disk image..."
	@objcopy -O binary tests.elf tests.bin
	@dd if=boot/debug_boot.bin of=test.img bs=512 count=1 conv=notrunc 2>/dev/null
	@dd if=tests.bin of=test.img seek=1 conv=notrunc 2>/dev/null
	@echo "Starting QEMU with test kernel..."
	@qemu-system-i386 -m 32M -drive file=test.img,format=raw,if=ide -vga std -display sdl -no-reboot

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
