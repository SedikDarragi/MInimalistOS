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
void timer_init(void);
void keyboard_init(void);
void memory_init(void);
void paging_init(void);
void syscall_init(void);
void usermode_init(void);
void ipc_init(void);
void network_init(void);
void vm_init(void);
void device_init(void);
void security_init(void);
void monitor_init(void);
void power_init(void);

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

/*
 * NOTE: This file appears to be a legacy or test kernel entry point.
 * The main, correct entry point is `kmain` in `kernel/kmain.c`.
 * The symptoms you are seeing (crash with "XLSKGP!") suggest that your
 * assembly code is still calling this `minimal_kmain` function instead of `kmain`.
 *
 * By commenting this function out, the build will fail with a linker error,
 * which will tell you exactly which file is trying to call it. You can then
 * edit that file (likely kernel/entry.s) to call `kmain` instead.
 */
/*
void minimal_kmain(void) {
    // This function is intentionally disabled to help find the build configuration error.
    // See the note above.
}
*/
