#include "../include/shell.h"
#include "../drivers/vga.h"
#include "../include/string.h"

// Global shell state definitions
shell_state_t shell_state;
command_history_t history;

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

void shell_run(void) {
    // Print prompt
    vga_print("\n");
    vga_print(shell_state.username);
    vga_print("@");
    vga_print(shell_state.hostname);
    vga_print(":");
    vga_print(shell_state.cwd);
    vga_print("$ ");
    
    // Note: Input loop would go here.
    // Since we don't have the keyboard driver details, we halt.
    while (1) {
        __asm__ volatile("hlt");
    }
}