#include "../include/shell.h"
#include "../drivers/vga.h"
#include "../include/string.h"
#include "io.h"
#include "../include/idt.h"

shell_state_t shell_state;
command_history_t history;

extern char keyboard_getchar(void);
extern void serial_putchar(uint16_t com, char c);
extern void enable_interrupts(void);
extern int vga_get_cursor_y(void);
extern void process_print_list(void);
extern uint32_t timer_get_ticks(void);

static void serial_print(const char* str) {
    // Temporarily disabled to prevent potential hangs on hardware polling
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
    // Drain any pending keyboard data to ensure IRQ 1 can trigger
    // Bit 0 of port 0x64 is the Output Buffer Full flag
    int timeout = 1000;
    while ((inb(0x64) & 0x01) && timeout-- > 0) {
        inb(0x60);
    }

    // Initialize state with safe defaults
    strcpy(shell_state.username, "Slime");
    strcpy(shell_state.hostname, "");
    strcpy(shell_state.cwd, "/");
    // Initialize history index but don't memset the whole thing yet
    history.count = 0;
}

void shell_execute_command(const char* command) {
    if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0) {
        shell_print("\nMinimalist OS Shell Commands:\n");
        shell_print("  help      - Show this help message\n");
        shell_print("  clear     - Clear the screen\n");
        shell_print("  ps        - List running processes\n");
        shell_print("  whoami    - Show current user\n");
        shell_print("  ver       - Show OS version\n");
        shell_print("  uptime    - Show system uptime (ticks)\n");
        shell_print("  testcmd   - Run test command\n");
    } else if (strcmp(command, "clear") == 0) {
        vga_clear();
    } else if (strcmp(command, "ps") == 0) {
        process_print_list();
    } else if (strcmp(command, "whoami") == 0) {
        shell_print(shell_state.username);
        shell_print("\n");
    } else if (strcmp(command, "ver") == 0) {
        shell_print("Minimalist OS v1.0\n");
    } else if (strcmp(command, "uptime") == 0) {
        char buf[32];
        shell_print("Uptime: ");
        itoa((int)timer_get_ticks(), buf, 10);
        shell_print(buf);
        shell_print(" ticks\n");
    } else if (strcmp(command, "testcmd") == 0) {
        shell_print("Test command executed!\n");
    } else if (strlen(command) > 0) {
        shell_print("Command not found\n");
    }
}

void shell_run(void) {
    char input_buffer[SHELL_BUFFER_SIZE];
    int buffer_pos = 0;
    char c;
    
    while (1) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        shell_print(shell_state.username);
        if (shell_state.hostname[0] != '\0') {
            shell_print("@");
            shell_print(shell_state.hostname);
        }
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
            if (c == 0) {
                c = shell_serial_getchar();
            }

            if (c == 0) {
                __asm__ volatile("pause"); 
                continue;
            }

            if (c == '\n' || c == '\r') {
                vga_putchar('\n');
                if (buffer_pos > 0) {
                    shell_execute_command(input_buffer);
                    shell_print("\n");
                }

                // Drain potential \n after \r (common CRLF sequence)
                if (c == '\r' && shell_serial_getchar() == '\n') {
                    // consumed
                }
                break;
            } else if (c == '\b' || (uint8_t)c == 0x7F) {
                if (buffer_pos > 0) {
                    buffer_pos--;
                    input_buffer[buffer_pos] = '\0';
                    char bs_str[2] = {'\b', '\0'};
                    shell_print(bs_str);
                }
            } else if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
                input_buffer[buffer_pos++] = c;
                char temp[2] = {c, '\0'};
                shell_print(temp);
            }
        }
    }
}