#include "../drivers/vga.h"
#include "../include/string.h"
#include "../kernel/log.h"
#include "../include/process_simple.h"
#include "../drivers/timer.h"
#include "../drivers/serial.h"
#include "../include/filesystem.h"
#include "../include/syscall.h"
#include "../drivers/keyboard_intl.h"
#include "../drivers/mouse.h"
#include "../include/program_loader.h"
#include "../include/net.h"
#include "../drivers/net_ne2k.h"

void kmain(void) {
    vga_clear();
    vga_print("Minimalist OS Kernel - Main Entry Point\n");
    
    log_info("Kernel main function started");
    
    // Initialize core systems
    vga_print("Initializing serial port...\n");
    if (serial_init()) {
        vga_print("Serial port: OK\n");
        serial_info("Serial port initialized for debugging");
    } else {
        vga_print("Serial port: FAILED\n");
    }
    
    vga_print("Initializing network core...\n");
    net_init();
    
    vga_print("Initializing NE2000 NIC (skeleton)...\n");
    if (net_ne2k_init() == 0) {
        vga_print("NE2000: registered as eth0 (skeleton)\n");
    } else {
        vga_print("NE2000: FAILED to register\n");
    }
    
    vga_print("Initializing process management...\n");
    process_init();
    
    vga_print("Initializing timer...\n");
    timer_init();
    
    vga_print("Initializing filesystem...\n");
    vfs_init();
    if (ramfs_mount() >= 0) {
        vga_print("RAM filesystem: OK\n");
        log_info("RAM filesystem mounted at /ram");
    } else {
        vga_print("RAM filesystem: FAILED\n");
    }
    
    vga_print("Initializing system call interface...\n");
    syscall_init();
    
    vga_print("Initializing keyboard (intl)...\n");
    if (keyboard_intl_init() == 0) {
        vga_print("Keyboard INTL: OK\n");
        log_info("International keyboard driver initialized");
        vga_print("Current layout: ");
        vga_print((char*)keyboard_intl_get_layout_name());
        vga_print("\n");
    } else {
        vga_print("Keyboard INTL: FAILED\n");
    }
    
    vga_print("Initializing mouse...\n");
    mouse_init();
    
    vga_print("Initializing program loader...\n");
    if (program_loader_init() == 0) {
        vga_print("Program Loader: OK\n");
        log_info("Program loader initialized");
    } else {
        vga_print("Program Loader: FAILED\n");
    }
    
    vga_print("Core systems initialized successfully\n");
    
    // Create a test process
    process_create(NULL, "test_process");
    
    vga_print("Process management: OK\n");
    vga_print("Timer: OK\n");
    vga_print("System ready!\n");
    
    // Simple kernel loop
    while (1) {
        // Kernel idle loop
        __asm__ volatile("hlt");
    }
}
