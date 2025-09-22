#include "../include/shell.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../fs/filesystem.h"
#include "../net/network.h"
#include "../ui/ui.h"
#include "process.h"

// Command implementations array
static const command_t commands[] = {
    {"help", cmd_help, "Show this help message"},
    {"clear", cmd_clear, "Clear the screen"},
    {"echo", cmd_echo, "Print arguments to the screen"},
    {"ls", cmd_ls, "List directory contents"},
    {"cat", cmd_cat, "Display file contents"},
    {"mkdir", cmd_mkdir, "Create a directory"},
    {"rm", cmd_rm, "Remove a file or directory"},
    {"cd", cmd_cd, "Change directory"},
    {"pwd", cmd_pwd, "Print working directory"},
    {"ps", cmd_ps, "List processes"},
    {"kill", cmd_kill, "Terminate a process"},
    {"ping", cmd_ping, "Ping a network host"},
    {"ifconfig", cmd_ifconfig, "Show network configuration"},
    {0, 0, 0}  // Null terminator
};

static char shell_buffer[SHELL_BUFFER_SIZE];
static int buffer_pos = 0;
static command_history_t history = {0};

// Tab completion
static void complete_command(const char* prefix) {
    int matches = 0;
    const char* match = NULL;
    int prefix_len = strlen(prefix);
    
    // Find matching commands
    for (int i = 0; commands[i].name; i++) {
        if (strncmp(prefix, commands[i].name, prefix_len) == 0) {
            matches++;
            match = commands[i].name;
        }
    }
    
    if (matches == 1 && match) {
        // Complete the command
        int match_len = strlen(match);
        for (int i = prefix_len; i < match_len; i++) {
            if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
                shell_buffer[buffer_pos++] = match[i];
                vga_putchar(match[i]);
            }
        }
    } else if (matches > 1) {
        // Show possible completions
        vga_print("\n");
        for (int i = 0; commands[i].name; i++) {
            if (strncmp(prefix, commands[i].name, prefix_len) == 0) {
                vga_print(commands[i].name);
                vga_print(" ");
            }
        }
        vga_print("\n$ ");
        vga_print(shell_buffer);
    }
}

// Command history functions
void shell_add_to_history(const char* command) {
    if (strlen(command) > 0) {
        strncpy(history.commands[history.count], command, SHELL_BUFFER_SIZE - 1);
        history.commands[history.count][SHELL_BUFFER_SIZE - 1] = '\0';
        history.count++;
        if (history.count >= MAX_HISTORY) {
            // Shift history down if we've reached max
            for (int i = 1; i < MAX_HISTORY; i++) {
                strncpy(history.commands[i-1], history.commands[i], SHELL_BUFFER_SIZE);
            }
            history.count--;
        }
        history.current = history.count;
    }
}

const char* shell_get_previous_command(void) {
    if (history.current > 0) {
        history.current--;
        return history.commands[history.current];
    }
    return NULL;
}

const char* shell_get_next_command(void) {
    if (history.current < history.count - 1) {
        history.current++;
        return history.commands[history.current];
    }
    history.current = history.count;
    return "";
}

void shell_init(void) {
    buffer_pos = 0;
    memset(shell_buffer, 0, SHELL_BUFFER_SIZE);
    vga_print("MinimalOS Shell v1.0\n");
    vga_print("Made by Sedik Darragi IMSET/CNI\n");
    vga_print("Type 'help' for available commands\n");
    vga_print("Use up/down arrows for command history\n");
    vga_print("Use Tab for command completion\n\n");
    
    // Initialize process management
    process_init();
    
    // Create initial processes
    process_create("shell", NULL);
}

void shell_run(void) {
    vga_print("$ ");
    
    while (1) {
        char c = keyboard_getchar();
        if (c == 0) continue;
        
        if (c == '\n') {
            vga_putchar('\n');
            shell_buffer[buffer_pos] = '\0';
            if (buffer_pos > 0) {
                shell_add_to_history(shell_buffer);
                shell_execute_command(shell_buffer);
            }
            buffer_pos = 0;
            vga_print("$ ");
        } 
        else if (c == '\b') {
            if (buffer_pos > 0) {
                buffer_pos--;
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            }
        }
        // Handle up arrow (previous command)
        else if (c == 0x48) {  // Up arrow
            const char* prev_cmd = shell_get_previous_command();
            if (prev_cmd) {
                // Clear current line
                for (int i = 0; i < buffer_pos; i++) {
                    vga_putchar('\b');
                    vga_putchar(' ');
                    vga_putchar('\b');
                }
                // Copy previous command to buffer
                strncpy(shell_buffer, prev_cmd, SHELL_BUFFER_SIZE - 1);
                buffer_pos = strlen(shell_buffer);
                vga_print(shell_buffer);
            }
        }
        // Handle down arrow (next command)
        else if (c == 0x50) {  // Down arrow
            const char* next_cmd = shell_get_next_command();
            // Clear current line
            for (int i = 0; i < buffer_pos; i++) {
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            }
            // Copy next command to buffer or clear
            if (next_cmd) {
                strncpy(shell_buffer, next_cmd, SHELL_BUFFER_SIZE - 1);
                buffer_pos = strlen(shell_buffer);
                vga_print(shell_buffer);
            } else {
                shell_buffer[0] = '\0';
                buffer_pos = 0;
            }
        }
        // Handle tab completion
        else if (c == '\t') {  // Tab key
            shell_buffer[buffer_pos] = '\0';
            complete_command(shell_buffer);
        }
        else if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
            shell_buffer[buffer_pos++] = c;
            vga_putchar(c);
        }
    }
}

static int parse_command(const char* command, char* argv[]) {
    int argc = 0;
    char* token = (char*)command;
    
    while (*token && argc < MAX_ARGS - 1) {
        // Skip whitespace
        while (*token == ' ' || *token == '\t') token++;
        if (*token == '\0') break;
        
        argv[argc++] = token;
        
        // Find end of token
        while (*token && *token != ' ' && *token != '\t') token++;
        if (*token) *token++ = '\0';
    }
    
    argv[argc] = 0;
    return argc;
}

void shell_execute_command(const char* command) {
    char cmd_copy[SHELL_BUFFER_SIZE];
    char* argv[MAX_ARGS];
    int argc;
    
    strcpy(cmd_copy, command);
    argc = parse_command(cmd_copy, argv);
    
    if (argc == 0) return;
    
    // Find and execute command
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].function(argc, argv);
            return;
        }
    }
    
    vga_print("Command not found: ");
    vga_print(argv[0]);
    vga_print("\n");
}

// Built-in command implementations
void cmd_help(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_print("Available commands:\n");
    for (int i = 0; commands[i].name; i++) {
        vga_print("  ");
        vga_print(commands[i].name);
        vga_print(" - ");
        vga_print(commands[i].description);
        vga_print("\n");
    }
}

void cmd_clear(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_clear();
}

void cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        vga_print(argv[i]);
        if (i < argc - 1) vga_print(" ");
    }
    vga_print("\n");
}

void cmd_ls(int argc, char* argv[]) {
    (void)argc; (void)argv;
    fs_list_directory(".");
}

void cmd_cat(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: cat <filename>\n");
        return;
    }
    fs_read_file(argv[1]);
}

void cmd_mkdir(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: mkdir <dirname>\n");
        return;
    }
    fs_create_directory(argv[1]);
}

void cmd_rm(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: rm <filename>\n");
        return;
    }
    fs_delete_file(argv[1]);
}

void cmd_cd(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: cd <directory>\n");
        return;
    }
    fs_change_directory(argv[1]);
}

void cmd_pwd(int argc, char* argv[]) {
    (void)argc; (void)argv;
    fs_print_working_directory();
}

void cmd_ps(int argc, char* argv[]) {
    (void)argc; (void)argv;
    process_print_list();
}

void cmd_kill(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: kill <pid>\n");
        return;
    }
    vga_print("Process ");
    vga_print(argv[1]);
    vga_print(" terminated\n");
}

void cmd_ping(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: ping <host>\n");
        return;
    }
    network_ping(argv[1]);
}

void cmd_ifconfig(int argc, char* argv[]) {
    (void)argc; (void)argv;
    network_show_config();
}
