#include "../include/shell.h"
#include "../drivers/vga.h"
#include "../include/string.h"
#include "io.h"

// Global shell state definitions
shell_state_t shell_state;
command_history_t history;

// External dependency for keyboard input (assuming standard driver naming)
extern char keyboard_getchar(void);

// Serial I/O helpers
static void serial_putc(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0); // Wait for transmit empty
    outb(0x3F8, c);
}

static void serial_print(const char* str) {
    while (*str) serial_putc(*str++);
}

static char serial_getchar(void) {
    if (inb(0x3F8 + 5) & 1) return inb(0x3F8); // Check data ready
    return 0;
}

static void shell_print(const char* str) {
    vga_print(str);
    serial_print(str);
}

void shell_init(void) {
    // Initialize shell state
    memset(&shell_state, 0, sizeof(shell_state_t));
    memset(&history, 0, sizeof(command_history_t));
    
    // Set default values
    strncpy(shell_state.username, "root", sizeof(shell_state.username) - 1);
    shell_state.username[sizeof(shell_state.username) - 1] = '\0';
    strncpy(shell_state.hostname, "minos", sizeof(shell_state.hostname) - 1);
    shell_state.hostname[sizeof(shell_state.hostname) - 1] = '\0';
    strncpy(shell_state.cwd, "/", sizeof(shell_state.cwd) - 1);
    shell_state.cwd[sizeof(shell_state.cwd) - 1] = '\0';
    
    // Print welcome message
    shell_print("\n");
    shell_print("Minimalist OS Shell v0.1\n");
    shell_print("Type 'help' for a list of commands.\n");
}

void shell_execute_command(const char* command) {
    if (strcmp(command, "help") == 0) {
        shell_print("Minimalist OS Shell Commands:\n");
        shell_print("  help     - Show this help message\n");
        shell_print("  clear    - Clear the screen\n");
        shell_print("  testcmd  - test command\n");
    } else if (strcmp(command, "clear") == 0) {
        vga_clear();
    } else if (strcmp(command, "testcmd") == 0) {
        shell_print("Test command executed!\n");
    } else if (strlen(command) > 0) {
        shell_print("Unknown command: ");
        shell_print(command); 
        shell_print("\n");
    }
}

void shell_run(void) {
    char input_buffer[SHELL_BUFFER_SIZE];
    int buffer_pos = 0;
    char c;
    
    while (1) {
        // Print prompt
        shell_print(shell_state.username);
        shell_print("@");
        shell_print(shell_state.hostname);
        shell_print(":");
        shell_print(shell_state.cwd);
        shell_print("$ ");
        
        buffer_pos = 0;
        memset(input_buffer, 0, SHELL_BUFFER_SIZE);
        
        // Input loop
        while (1) {
            c = keyboard_getchar();
            if (c == 0) c = shell_serial_getchar(); // Also check serial input
            
            if (c == 0) {
                __asm__ volatile("hlt"); // Wait for interrupt to save CPU
                continue;
            }
            
            if (c == '\n') {
                shell_print("\n");
                shell_execute_command(input_buffer);
                break; // Break input loop to reprint prompt
            } else if (c == '\b') {
                if (buffer_pos > 0) {
                    buffer_pos--;
                    input_buffer[buffer_pos] = '\0';
                    shell_print("\b \b"); // Visual backspace
                }
            } else if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
                input_buffer[buffer_pos++] = c;
                char temp[2] = {c, '\0'};
                shell_print(temp);
            }
        }
    }
}