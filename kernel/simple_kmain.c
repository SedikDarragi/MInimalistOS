#include "../drivers/vga.h"
#include "../include/string.h"
#include "../kernel/log.h"

void kmain(void) {
    // Write to VGA using direct memory access
    unsigned short *vga_buffer = (unsigned short *)0xB8000;
    
    // Write "Hello, OS!" to VGA
    vga_buffer[0] = 'H' | (0x0F << 8);
    vga_buffer[1] = 'e' | (0x0F << 8);
    vga_buffer[2] = 'l' | (0x0F << 8);
    vga_buffer[3] = 'l' | (0x0F << 8);
    vga_buffer[4] = 'o' | (0x0F << 8);
    vga_buffer[5] = ',' | (0x0F << 8);
    vga_buffer[6] = ' ' | (0x0F << 8);
    vga_buffer[7] = 'O' | (0x0F << 8);
    vga_buffer[8] = 'S' | (0x0F << 8);
    vga_buffer[9] = '!' | (0x0F << 8);
    
    log_info("Kernel main function started");
    
    vga_print("Kernel initialization complete!\n");
    
    // Main loop
    while (1) {
        // Do nothing - just wait
    }
    
    // Should never reach here
    vga_print("This should never be printed!\n");
}