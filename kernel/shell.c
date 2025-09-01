#include "shell.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../fs/filesystem.h"
#include "../net/network.h"
#include "../ui/ui.h"

static char shell_buffer[SHELL_BUFFER_SIZE];
static int buffer_pos = 0;

typedef struct {
    const char* name;
    void (*function)(int argc, char* argv[]);
    const char* description;
} command_t;

static command_t commands[] = {
    {"help", cmd_help, "Show available commands"},
    {"clear", cmd_clear, "Clear the screen"},
    {"echo", cmd_echo, "Print text to screen"},
    {"ls", cmd_ls, "List directory contents"},
    {"cat", cmd_cat, "Display file contents"},
    {"mkdir", cmd_mkdir, "Create directory"},
    {"rm", cmd_rm, "Remove file or directory"},
    {"cd", cmd_cd, "Change directory"},
    {"pwd", cmd_pwd, "Print working directory"},
    {"ps", cmd_ps, "List running processes"},
    {"kill", cmd_kill, "Kill a process"},
    {"ping", cmd_ping, "Ping a network host"},
    {"ifconfig", cmd_ifconfig, "Configure network interface"},
    {0, 0, 0}
};

void shell_init(void) {
    buffer_pos = 0;
    memset(shell_buffer, 0, SHELL_BUFFER_SIZE);
    vga_print("MinimalOS Shell v1.0\n");
    vga_print("Made by Sedik Darragi IMSET/CNI\n");
    vga_print("Type 'help' for available commands\n\n");
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
                shell_execute_command(shell_buffer);
            }
            buffer_pos = 0;
            vga_print("$ ");
        } else if (c == '\b') {
            if (buffer_pos > 0) {
                buffer_pos--;
                vga_putchar('\b');
            }
        } else if (buffer_pos < SHELL_BUFFER_SIZE - 1) {
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
    vga_print("PID  NAME\n");
    vga_print("1    kernel\n");
    vga_print("2    shell\n");
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
