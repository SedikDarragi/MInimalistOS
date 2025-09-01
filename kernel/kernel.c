#include "kernel.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../fs/filesystem.h"
#include "../net/network.h"
#include "../ui/ui.h"
#include "shell.h"

void __attribute__((noreturn)) kmain(void) {
    // Initialize VGA display immediately
    vga_init();
    vga_clear();
    
    // Print immediate debug message
    vga_print("KERNEL STARTED!\n");
    vga_print("MinimalOS v1.0\n");
    vga_print("Initializing system...\n");
    
    // Initialize drivers
    keyboard_init();
    timer_init();
    
    // Initialize filesystem
    fs_init();
    vga_print("Filesystem initialized\n");
    
    // Initialize network stack
    network_init();
    vga_print("Network stack initialized\n");
    
    // Initialize UI system
    ui_init();
    vga_print("UI system initialized\n");
    
    // Start shell
    vga_print("Starting shell...\n\n");
    shell_init();
    
    // Main kernel loop
    while (1) {
        shell_run();
    }
}
