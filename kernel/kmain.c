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
#include "../include/pci.h"

extern void shell_init(void);
extern void shell_run(void);

void kmain(void) {
    /* Don't clear screen immediately - let debug chars stay visible */
    /* vga_clear(); */
    vga_print("\nMinimalist OS Kernel - Main Entry Point\n");
    
    log_info("Kernel main function started");
    
    // Initialize core systems
    vga_print("Initializing serial port...\n");
    if (serial_init()) {
        vga_print("Serial port: OK\n");
        serial_info("Serial port initialized for debugging");
    } else {
        vga_print("Serial port: FAILED\n");
    }

    // Temporary: dump PCI bus 0 so we can see NIC vendor/device IDs
    pci_dump_bus0();
    
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
    // vfs_init();  // Not included in simple kernel build
    // if (ramfs_mount() >= 0) {
    //     vga_print("RAM filesystem: OK\n");
    //     log_info("RAM filesystem mounted at /ram");
    // } else {
    //     vga_print("RAM filesystem: FAILED\n");
    // }
    vga_print("Filesystem: SKIPPED (simple build)\n");
    
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
    // process_create("test_process", NULL);
    
    vga_print("Process management: OK\n");
    vga_print("Timer: OK\n");
    vga_print("System ready!\n");
    
    // Initialize and run the shell
    shell_init();
    shell_run();

    // Simple kernel loop
    while (1) {
        // Kernel idle loop
        __asm__ volatile("hlt");
    }
}

// Stubs for shell if not linked
void shell_init(void) {
    vga_print("Shell not available (linking error)\n");
}
void shell_run(void) {
    while(1) __asm__ volatile("hlt");
}
