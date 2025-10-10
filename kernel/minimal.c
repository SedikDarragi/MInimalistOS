#include <stdint.h>
#include <stddef.h>

// Forward declarations for shell functions
void shell_init(void);
void shell_run(void);

// Kernel entry point
void kmain(void) {
    // Initialize and run the shell
    shell_init();
    shell_run();
    
    // Halt the CPU if shell returns (shouldn't happen)
    while (1) {
        __asm__ volatile ("hlt");
    }
}
