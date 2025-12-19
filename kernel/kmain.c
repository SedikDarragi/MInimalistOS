#include "../drivers/vga.h"
#include "../include/string.h"
#include "../kernel/log.h"
#include "../include/process_simple.h"
#include "../drivers/timer.h"

void kmain(void) {
    vga_clear();
    vga_print("Minimalist OS Kernel - Main Entry Point\n");
    
    log_info("Kernel main function started");
    
    // Initialize core systems
    vga_print("Initializing process management...\n");
    process_init();
    
    vga_print("Initializing timer...\n");
    timer_init();
    
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
