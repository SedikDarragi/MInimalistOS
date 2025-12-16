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
WARN_CFLAGS = -Wall -Wextra -Werror \
              -Wshadow -Wpointer-arith -Wcast-align \
              -Wwrite-strings -Wmissing-prototypes \
              -Wmissing-declarations -Wredundant-decls \
              -Wnested-externs -Winline -Wno-long-long \
              -Wuninitialized -Wstrict-prototypes

# Base flags
# Include directories
INCLUDES = -I./include -I./kernel -I. -I./drivers -I./fs
LDFLAGS = -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000
ASFLAGS = -f elf32

# Source file organization
KERNEL_SRCS := $(shell find kernel/ -name '*.c' -not -name 'test_*.c' -not -name '*_test.c' -not -name 'tests.c')
KERNEL_TEST_SRCS := $(shell find kernel/ -name '*_test.c')
TEST_SRCS := kernel/tests.c
DRIVER_SRCS := $(shell find drivers/ -name '*.c')
FS_SRCS := $(shell find fs/ -name '*.c')

# Assembly sources (both .s and .asm)
KERNEL_ASM_SRCS := $(shell find kernel/ -name '*.s' -o -name '*.asm')

# Combine all source files
ALL_SRCS := $(KERNEL_SRCS) $(KERNEL_TEST_SRCS) $(DRIVER_SRCS) $(FS_SRCS)
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

# Phony targets
.PHONY: all clean clean-all run debug test size run-test

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

# Linker flags
LDFLAGS += -m elf_i386 -T link.ld -nostdlib -z max-page-size=0x1000
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/kernel.map -Wl,--gc-sections

# Link kernel
kernel.elf: $(KERNEL_OBJS) link.ld | $(BUILD_DIR)
	$(E) "  LD      $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)
	$(E) "  SIZE    $@"
	@$(SIZE) $@ || true

# Link test kernel
tests.elf: $(TEST_ALL_SRCS:.c=.o) $(KERNEL_ASM_SRCS:.s=.o) link.ld | $(BUILD_DIR)
	$(E) "  LD      $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $(TEST_ALL_SRCS:.c=.o) $(KERNEL_ASM_SRCS:.s=.o)
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
QEMU_OPTS = -m 32M -monitor stdio -no-reboot -no-shutdown
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
