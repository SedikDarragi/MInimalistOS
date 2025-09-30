#include <stdint.h>
#include <stddef.h>

// VGA text mode memory address
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

// Kernel entry point
void kmain(void) {
    // Clear the screen with blue background
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEMORY[i] = 0x1F00 | ' ';  // Blue background, white space
    }
    
    // Print a message
    const char* message = "Hello from MinimalOS Kernel!";
    const size_t message_length = 28;  // Length of the message
    
    // Center the message on the screen
    size_t start_x = (VGA_WIDTH - message_length) / 2;
    size_t start_y = 12;  // Middle row
    
    for (size_t i = 0; i < message_length; i++) {
        const size_t index = start_y * VGA_WIDTH + start_x + i;
        VGA_MEMORY[index] = 0x1F00 | message[i];  // White on blue
    }
    
    // Halt the CPU
    while (1) {
        __asm__ volatile ("hlt");
    }
}
