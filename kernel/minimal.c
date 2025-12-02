#include <stdint.h>
#include <stddef.h>

// VGA text buffer
#define VGA_BUFFER ((volatile uint16_t*)0xB8000)

// Forward declarations
void shell_init(void);
void shell_run(void);
void idt_init(void);
void fs_init(void);
void process_init(void);

// Simple delay function
static void delay(int count) {
    for (volatile int i = 0; i < count; i++);
}

// Function to write a character to VGA memory
static void vga_putc(int x, int y, char c, uint8_t color) {
    VGA_BUFFER[y * 80 + x] = (color << 8) | c;
}

// Function to print a string to VGA memory
static void vga_puts(int x, int y, const char *str, uint8_t color) {
    while (*str) {
        vga_putc(x++, y, *str++, color);
        if (x >= 80) { x = 0; y++; }
    }
}

// Kernel entry point
void kmain(void) {
    // Debug: Write to serial port - DISABLED (causes triple fault)
    // volatile char *serial = (volatile char*)0x3F8;
    // *serial = 'C';  // C for C code
    
    // Don't clear screen immediately - let's see if the assembly pattern is visible
    // Clear screen with black background
    // for (int i = 0; i < 80*25; i++) {
    //     VGA_BUFFER[i] = 0x0F20;  // Black background, white text, space character
    // }
    
    // Print debug message at top of screen
    vga_puts(0, 0, "KERNEL STARTED", 0x0F);
    
    // Test VGA output at different positions
    vga_puts(0, 2, "Testing VGA output...", 0x0A);
    
    // Test different colors
    vga_puts(0, 4, "Red text", 0x04);
    vga_puts(0, 5, "Green text", 0x02);
    vga_puts(0, 6, "Blue text", 0x01);
    
    // Wait a bit to see the message
    delay(10000000);
    
    // Clear the screen to blue background
    for (int i = 0; i < 80*25; i++) {
        VGA_BUFFER[i] = 0x1F00;  // Blue background, space character
    }
    
    // Print a message after clearing
    vga_puts(0, 0, "INITIALIZING INTERRUPTS...", 0x1F);
    
    // Initialize interrupts
    idt_init();
    
    // Print a message after IDT init
    vga_puts(0, 2, "INTERRUPTS INITIALIZED", 0x1F);
    
    // Initialize filesystem
    vga_puts(0, 3, "INITIALIZING FILESYSTEM...", 0x1F);
    fs_init();
    vga_puts(0, 4, "FILESYSTEM READY", 0x1F);
    
    // Initialize process system
    vga_puts(0, 5, "INITIALIZING PROCESSES...", 0x1F);
    process_init();
    vga_puts(0, 6, "PROCESS SYSTEM READY", 0x1F);

    // Initialize and run the shell
    shell_init();
    shell_run();
    
    // Halt the CPU if shell returns (shouldn't happen)
    while (1) {
        __asm__ volatile ("hlt");
    }
}
