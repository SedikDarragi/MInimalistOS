#include <stdint.h>
#include <stddef.h>

// VGA text buffer
#define VGA_BUFFER ((volatile uint16_t*)0xB8000)

// Forward declarations for shell functions
// Forward declarations
void shell_init(void);
void shell_run(void);
void idt_init(void);

// Simple delay function
static void delay(int count) {
    for (volatile int i = 0; i < count; i++);
}

// Kernel entry point
void kmain(void) {
    // Test VGA output - this should be visible immediately
    const char *test_msg = "MINIMAL OS IS RUNNING";
    for (int i = 0; test_msg[i]; i++) {
        VGA_BUFFER[i] = 0x1F00 | test_msg[i];  // White on blue
    }
    
    // Wait a bit to see the message
    delay(5000000);
    
    // Clear the screen
    for (int i = 0; i < 80*25; i++) {
        VGA_BUFFER[i] = 0x1F00;  // Clear to blue background
    }
    
    VGA_BUFFER[0] = 0x4F00 | 'A'; // White on Red 'A'

    // Initialize interrupts
    idt_init();

    VGA_BUFFER[1] = 0x4F00 | 'B'; // White on Red 'B'

    // Initialize and run the shell
    shell_init();
    shell_run();
    
    // Halt the CPU if shell returns (shouldn't happen)
    while (1) {
        __asm__ volatile ("hlt");
    }
}
