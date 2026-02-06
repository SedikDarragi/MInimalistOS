#include "../drivers/vga.h"
#include "../include/string.h"
#include "../kernel/log.h"

void kmain(void) {
    // Write "Hello, OS!" to VGA
    vga_print("Hello, OS!\n");
    
    log_info("Kernel main function started");
    
    vga_print("Kernel initialization complete!\n");
    
    // Main loop
    while (1) {
        // Do nothing - just wait
    }
    
    // Should never reach here
    vga_print("This should never be printed!\n");
}