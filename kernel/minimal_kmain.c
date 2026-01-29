/* Minimal kernel for testing */

#include "../drivers/vga.h"

void kmain(void) {
    vga_print("Kernel loaded successfully!\n");
    vga_print("Entering infinite loop...\n");
    
    while(1) {
        /* Infinite loop to keep kernel running */
    }
}