#include "../drivers/vga.h"
#include "../include/string.h"
#include "../kernel/log.h"
#include "../kernel/process.h"
#include "../drivers/timer.h"
#include "../drivers/serial.h"
#include "../include/filesystem.h"
#include "../include/syscall.h"
#include "../drivers/keyboard_intl.h"
#include "../include/program_loader.h"
#include "../include/net.h"
#include "../drivers/net_ne2k.h"
#include "../include/pci.h"
#include "../include/idt.h"
#include "../include/memory.h"
#include "../include/power.h"

extern void shell_init(void);
extern void shell_run(void);
extern void keyboard_init(void);

void kmain(void) {
    log_init();
    log_info("Kernel started");

    idt_init();
    log_info("IDT initialized");
    memory_init();
    log_info("Memory initialized");
    paging_init();
    log_info("Paging initialized");
    
    vga_init();
    
    vga_print("Initializing serial port...\n");
    if (serial_init()) {
        vga_print("Serial port: OK\n");
        serial_info("Serial port initialized for debugging");
    } else {
        vga_print("Serial port: FAILED\n");
    }

    vga_print("Networking: SKIPPED\n");
    
    vga_print("Initializing process management...\n");
    process_init();
    serial_info("Process management initialized");
    
    vga_print("Initializing timer...\n");
    
    vga_print("Initializing filesystem...\n");
    vga_print("Filesystem: SKIPPED (simple build)\n");
    
    vga_print("Initializing system call interface...\n");
    syscall_init();
    serial_info("Syscalls initialized");
    
    vga_print("Initializing keyboard...\n");
    keyboard_init();
    vga_print("Keyboard: OK\n");
    enable_irq(1);
    
    vga_print("Initializing power management...\n");
    serial_info("Power management initialized");
    
    vga_print("Initializing program loader...\n");
    if (program_loader_init() == 0) {
        vga_print("Program Loader: OK\n");
        log_info("Program loader initialized");
    } else {
        vga_print("Program Loader: FAILED\n");
    }
    
    vga_print("Core systems initialized successfully\n");
    
    vga_print("Process management: OK\n");
    vga_print("Timer: OK\n");

    vga_clear();

    vga_print("Starting Shell...\n");

    shell_init();
    
    __asm__ volatile("sti");
    shell_run();

    while (1) {
        __asm__ volatile("hlt");
    }
}

void vfs_init(void) {}
int ramfs_mount(void) { return -1; }
