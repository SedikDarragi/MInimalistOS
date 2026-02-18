#include "../include/shell.h"
#include "../drivers/vga.h"
#include "../include/string.h"

// Global shell state definitions
shell_state_t shell_state;
command_history_t history;

// External dependency for keyboard input (assuming standard driver naming)
extern char keyboard_getchar(void);

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
    vga_print("\n");
    vga_print("Minimalist OS Shell v0.1\n");
    vga_print("Type 'help' for a list of commands.\n");
}

void shell_execute_command(const char* command) {
    if (strcmp(command, "help") == 0) {
        vga_print("Minimalist OS Shell Commands:\n");
        vga_print("  help     - Show this help message\n");
        vga_print("  clear    - Clear the screen\n");
    } else if (strcmp(command, "clear") == 0) {
        vga_clear();
    } else if (strlen(command) > 0) {
        vga_print("Unknown command: ");
        vga_print(command);
        vga_print("\n");
    }
}

void shell_run(void) {
    char input_buffer[SHELL_BUFFER_SIZE];
    int buffer_pos = 0;
    char c;
    
    while (1) {
        // Print prompt
        vga_print(shell_state.username);
        vga_print("@");
        vga_print(shell_state.hostname);
        vga_print(":");
        vga_print(shell_state.cwd);
        vga_print("$ ");
        
        buffer_pos = 0;
        memset(input_buffer, 0, SHELL_BUFFER_SIZE);
        
        // Input loop
        while (1) {
            c = keyboard_getchar();
            
            if (c == 0) continue; // No key pressed
            
            if (c == '\n') {
                vga_print("\n");
                shell_execute_command(input_buffer);
                break; // Break input loop to reprint prompt
            } else if (c == '\b') {
                if (buffer_pos > 0) {
                    buffer_pos--;
                    input_buffer[buffer_pos] = '\0';
                    vga_print("\b \b"); // Visual backspace
                }
            } else if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
                input_buffer[buffer_pos++] = c;
                char temp[2] = {c, '\0'};
                vga_print(temp);
            }
        }
    }
}