#include "../include/shell.h"
#include "../drivers/vga.h"
#include "../include/string.h"
#include "io.h"

shell_state_t shell_state;
command_history_t history;

extern char keyboard_getchar(void);
extern void serial_putchar(uint16_t com, char c);

static void serial_print(const char* str) {
    while (*str) {
        if (*str == '\n') serial_putchar(0x3F8, '\r');
        serial_putchar(0x3F8, *str++);
    }
}

static char shell_serial_getchar(void) {
    if (inb(0x3F8 + 5) & 1) return inb(0x3F8);
    return 0;
}

static void shell_print(const char* str) {
    vga_print(str);
    serial_print(str);
}

void shell_init(void) {
    memset(&shell_state, 0, sizeof(shell_state_t));
    memset(&history, 0, sizeof(command_history_t));
    
    strncpy(shell_state.username, "root", sizeof(shell_state.username) - 1);
    shell_state.username[sizeof(shell_state.username) - 1] = '\0';
    strncpy(shell_state.hostname, "minos", sizeof(shell_state.hostname) - 1);
    shell_state.hostname[sizeof(shell_state.hostname) - 1] = '\0';
    strncpy(shell_state.cwd, "/", sizeof(shell_state.cwd) - 1);
    shell_state.cwd[sizeof(shell_state.cwd) - 1] = '\0';
    
    shell_print("\n");
    shell_print("Minimalist OS Shell v0.1\n");
    shell_print("Type 'help' for a list of commands.\n");
    shell_print("Ready.\n");
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
    
    // Ensure VGA cursor is visible and updated
    vga_set_cursor(0, 11); // Set to a fresh line below kmain output
    
    vga_print("\n[SHELL] Starting interactive loop...\n");
    serial_print("[SHELL] Starting interactive loop...\n");

    while (1) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        shell_print(shell_state.username);
        shell_print("@");
        shell_print(shell_state.hostname);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        shell_print(":");
        vga_set_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
        shell_print(shell_state.cwd);
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        shell_print("$ ");
        
        buffer_pos = 0;
        memset(input_buffer, 0, SHELL_BUFFER_SIZE);
        
        while (1) {
            c = keyboard_getchar();
            if (c == 0) c = shell_serial_getchar();
            
            if (c == 0) continue;
            
            if (c == '\n') {
                shell_print("\n");
                shell_execute_command(input_buffer);
                break;
            } else if (c == '\b') {
                if (buffer_pos > 0) {
                    buffer_pos--;
                    input_buffer[buffer_pos] = '\0';
                    shell_print("\b \b");
                }
            } else if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
                input_buffer[buffer_pos++] = c;
                char temp[2] = {c, '\0'};
                shell_print(temp);
            }
        }
    }
}