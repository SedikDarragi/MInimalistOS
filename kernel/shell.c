#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

// Simple VGA text mode implementation
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x0F; // White on black

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

static void vga_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            // Scroll up
            for (size_t y = 1; y < VGA_HEIGHT; y++) {
                for (size_t x = 0; x < VGA_WIDTH; x++) {
                    VGA_MEMORY[(y-1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
                }
            }
            // Clear bottom line
            for (size_t x = 0; x < VGA_WIDTH; x++) {
                VGA_MEMORY[(VGA_HEIGHT-1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
            }
            terminal_row = VGA_HEIGHT - 1;
        }
    } else {
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        VGA_MEMORY[index] = vga_entry(c, terminal_color);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_row = 0;
            }
        }
    }
}

static void vga_print(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        vga_putchar(data[i]);
    }
}

static void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

// Simple keyboard input (simulated for now)
static char keyboard_getchar(void) {
    // In a real implementation, this would read from the keyboard controller
    // For now, we'll simulate a simple command sequence
    static const char* test_input = "help\necho Hello, World!\nclear\n";
    static size_t pos = 0;
    
    // Return the next character from our test input
    if (test_input[pos] == '\0') {
        pos = 0; // Loop the test input
    }
    return test_input[pos++];
}

// Simple string functions
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

static int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Command structure
typedef struct {
    const char* name;
    void (*func)(int argc, char** argv);
    const char* help;
} command_t;

// Command implementations
static void cmd_help(int argc, char** argv) {
    vga_print("Available commands:\n");
    vga_print("  help    - Show this help message\n");
    vga_print("  echo    - Print a test message\n");
    vga_print("  clear   - Clear the screen\n");
}

static void cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        vga_print(argv[i]);
        if (i < argc - 1) {
            vga_print(" ");
        }
    }
    vga_print("\n");
}

static void cmd_clear(int argc, char** argv) {
    vga_clear();
}

// Command table
static const command_t commands[] = {
    {"help", cmd_help, "Show this help message"},
    {"echo", cmd_echo, "Print a test message"},
    {"clear", cmd_clear, "Clear the screen"},
    {NULL, NULL, NULL} // Sentinel
};

// Find a command by name
static const command_t* find_command(const char* name) {
    for (const command_t* cmd = commands; cmd->name != NULL; cmd++) {
        if (strcmp(cmd->name, name) == 0) {
            return cmd;
        }
    }
    return NULL;
}

// Execute a command
static void execute_command(const char* line) {
    char* args[32];
    int argc = 0;
    char* saveptr;
    char* token = strtok_r((char*)line, " \t\n", &saveptr);
    
    // Parse arguments
    while (token != NULL && argc < 31) {
        args[argc++] = token;
        token = strtok_r(NULL, " \t\n", &saveptr);
    }
    args[argc] = NULL;
    
    if (argc == 0) {
        return; // Empty command
    }
    
    // Find and execute the command
    const command_t* cmd = find_command(args[0]);
    if (cmd != NULL) {
        cmd->func(argc, args);
    } else {
        vga_print("Command not found: ");
        vga_print(args[0]);
        vga_print("\n");
    }
}

// Shell main function
void shell_run(void) {
    char input[256];
    size_t pos = 0;
    
    vga_print("MinimalOS Shell\n");
    vga_print("Type 'help' for a list of commands\n");
    
    while (1) {
        vga_print("> ");
        
        // Read a line of input
        pos = 0;
        while (1) {
            char c = keyboard_getchar();
            
            if (c == '\n' || c == '\r') {
                vga_putchar('\n');
                input[pos] = '\0';
                break;
            } else if ((c == '\b' || c == 127) && pos > 0) { // Backspace or Delete
                pos--;
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            } else if (pos < sizeof(input) - 1 && c >= 32 && c <= 126) {
                // Only accept printable ASCII characters
                input[pos++] = c;
                vga_putchar(c);
            }
        }
        
        // Execute the command
        if (pos > 0) {
            execute_command(input);
        }
    }
}

// Shell initialization
void shell_init(void) {
    vga_clear();
    vga_print("MinimalOS Shell Initialized\n");
#include "../kernel/utils.h"

// Forward declarations for VGA functions if not in header
#ifndef VGA_HAS_VPRINTF
extern void vga_printf(const char* format, ...);
extern void vga_vprintf(const char* format, va_list args);
#endif

// Global shell state
shell_state_t shell_state = {0};

// Command implementations array
static command_t builtin_commands[] = {
    {"help", cmd_help, "Show this help message", 1, 2},
    {"clear", cmd_clear, "Clear the screen", 1, 1},
    {"echo", cmd_echo, "Print arguments to the screen", 1, 0},
    {"ls", cmd_ls, "List directory contents", 1, 2},
    {"cat", cmd_cat, "Display file contents", 2, 2},
    {"mkdir", cmd_mkdir, "Create a directory", 2, 2},
    {"rm", cmd_rm, "Remove a file or directory", 2, 2},
    {"cd", cmd_cd, "Change directory", 1, 2},
    {"pwd", cmd_pwd, "Print working directory", 1, 1},
    {"ps", cmd_ps, "List processes", 1, 1},
    {"kill", cmd_kill, "Terminate a process by PID", 2, 2},
    {"ping", cmd_ping, "Ping a network host", 2, 2},
    {"ifconfig", cmd_ifconfig, "Show network configuration", 1, 1},
    {"alias", cmd_alias, "Manage command aliases", 1, 0},
    {"unalias", cmd_unalias, "Remove command aliases", 2, 2},
    {"history", cmd_history, "Show command history", 1, 2},
    {"export", cmd_export, "Set environment variables", 2, 3},
    {0, 0, 0, 0, 0}  // Null terminator
};

static char shell_buffer[SHELL_BUFFER_SIZE];
static int buffer_pos = 0;
command_history_t history = {0};
static char current_prompt[64] = "minios> ";

// ==================== Tab Completion ====================
static void complete_command(const char* prefix) {
    int matches = 0;
    const char* match = NULL;
    int prefix_len = strlen(prefix);
    
    // Find matching commands
    for (int i = 0; builtin_commands[i].name; i++) {
        if (strncmp(prefix, builtin_commands[i].name, prefix_len) == 0) {
            matches++;
            match = builtin_commands[i].name;
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
        vga_print(ANSI_COLOR_CYAN "\n");
        for (int i = 0; builtin_commands[i].name; i++) {
            if (strncmp(prefix, builtin_commands[i].name, prefix_len) == 0) {
                vga_print(builtin_commands[i].name);
                vga_print(" ");
            }
        }
        vga_print(ANSI_COLOR_RESET "\n");
        vga_print(current_prompt);
        vga_print(shell_buffer);
    }
}

// ==================== Command History ====================
void shell_add_to_history(const char* command) {
    if (!command || !*command) return;
    
    // Don't add duplicate consecutive commands
    if (history.count > 0 && strcmp(history.commands[history.count-1], command) == 0) {
        return;
    }
    
    strncpy(history.commands[history.count], command, SHELL_BUFFER_SIZE - 1);
    history.commands[history.count][SHELL_BUFFER_SIZE - 1] = '\0';
    
    if (++history.count >= MAX_HISTORY) {
        // Shift history down if we've reached max
        for (int i = 1; i < MAX_HISTORY; i++) {
            strcpy(history.commands[i-1], history.commands[i]);
        }
        history.count = MAX_HISTORY - 1;
    }
    
    history.current = history.count;
    history.saved_pos = buffer_pos;
}

const char* shell_get_previous_command(void) {
    if (history.current > 0) {
        if (history.current == history.count) {
            // Save the current line before navigating history
            strncpy(history.commands[history.count], shell_buffer, SHELL_BUFFER_SIZE);
            history.saved_pos = buffer_pos;
        }
        history.current--;
        return history.commands[history.current];
    }
    return NULL;
}

const char* shell_get_next_command(void) {
    if (history.current < history.count - 1) {
        history.current++;
        return history.commands[history.current];
    } else if (history.current == history.count - 1) {
        history.current++;
        return history.commands[history.count]; // Return the saved current line
    }
    return NULL;
}

// ==================== Alias Management ====================
int shell_add_alias(const char* name, const char* value) {
    if (!name || !*name || !value || shell_state.alias_count >= MAX_ALIASES) {
        return -1;
    }
    
    // Check if alias already exists
    for (int i = 0; i < shell_state.alias_count; i++) {
        if (strcmp(shell_state.aliases[i].name, name) == 0) {
            strncpy(shell_state.aliases[i].value, value, SHELL_BUFFER_SIZE - 1);
            shell_state.aliases[i].value[SHELL_BUFFER_SIZE - 1] = '\0';
            return 0;
        }
    }
    
    // Add new alias
    strncpy(shell_state.aliases[shell_state.alias_count].name, name, 31);
    shell_state.aliases[shell_state.alias_count].name[31] = '\0';
    strncpy(shell_state.aliases[shell_state.alias_count].value, value, SHELL_BUFFER_SIZE - 1);
    shell_state.aliases[shell_state.alias_count].value[SHELL_BUFFER_SIZE - 1] = '\0';
    shell_state.alias_count++;
    
    return 0;
}

char* shell_expand_aliases(const char* input) {
    static char expanded[SHELL_BUFFER_SIZE];
    char command[SHELL_BUFFER_SIZE];
    
    // Extract the first word (command)
    const char* p = input;
    while (*p && *p <= ' ') p++; // Skip leading whitespace
    
    const char* cmd_start = p;
    while (*p && *p > ' ') p++;
    
    int cmd_len = p - cmd_start;
    if (cmd_len == 0) return (char*)input;
    
    strncpy(command, cmd_start, cmd_len);
    command[cmd_len] = '\0';
    
    // Check if it's an alias
    for (int i = 0; i < shell_state.alias_count; i++) {
        if (strcmp(command, shell_state.aliases[i].name) == 0) {
            // Found a matching alias, expand it
            strncpy(expanded, shell_state.aliases[i].value, SHELL_BUFFER_SIZE - 1);
            expanded[SHELL_BUFFER_SIZE - 1] = '\0';
            
            // Append the rest of the input if any
            while (*p && *p <= ' ') p++; // Skip whitespace
            if (*p) {
                strncat(expanded, " ", SHELL_BUFFER_SIZE - strlen(expanded) - 1);
                strncat(expanded, p, SHELL_BUFFER_SIZE - strlen(expanded) - 1);
            }
            
            return expanded;
        }
    }
    
    return (char*)input; // No alias found, return original
}

// ==================== Command Registration ====================
void shell_register_command(const command_t* cmd) {
    // Find the first empty slot in builtin_commands
    int i;
    for (i = 0; builtin_commands[i].name != NULL; i++) {
        if (i >= MAX_ARGS - 2) return; // No space left
    }
    
    // Copy the command
    memcpy(&builtin_commands[i], cmd, sizeof(command_t));
    
    // Add a new terminator
    if (i < MAX_ARGS - 2) {
        memset(&builtin_commands[i+1], 0, sizeof(command_t));
    }
}

// ==================== Prompt Management ====================
void shell_set_prompt(const char* prompt) {
    if (prompt) {
        strncpy(current_prompt, prompt, sizeof(current_prompt) - 1);
        current_prompt[sizeof(current_prompt) - 1] = '\0';
    }
}

// ==================== Input/Output Helpers ====================
void shell_print_error(const char* format, ...) {
    va_list args;
    (void)args; // Mark as unused for now
    
    vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
    vga_print("Error: ");
    vga_print((char*)format);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void shell_print_info(const char* format, ...) {
    va_list args;
    (void)args; // Mark as unused for now
    vga_set_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);
    vga_print("Info: ");
    vga_print((char*)format);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}

void shell_print_success(const char* format, ...) {
    va_list args;
    (void)args; // Mark as unused for now
    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_print("Success: ");
    vga_print((char*)format);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
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

// Simple command parsing function
static int parse_command(char* command, char* argv[]) {
    int argc = 0;
    char* token = strtok(command, " \t");
    
    while (token && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
        if (*token) *token++ = '\0';
    }
    
    argv[argc] = 0;
    return argc;
}

void shell_execute_command(const char* command) {
    if (!command || !*command || command[0] == '#') {
        return; // Skip empty lines and comments
    }
    
    // Add to history (except if it's the same as the last command)
    if (history.count == 0 || strcmp(command, history.commands[history.count-1]) != 0) {
        shell_add_to_history(command);
    }
    
    // Make a copy we can modify
    char cmd_copy[SHELL_BUFFER_SIZE];
    strncpy(cmd_copy, command, SHELL_BUFFER_SIZE - 1);
    cmd_copy[SHELL_BUFFER_SIZE - 1] = '\0';
    
    // Parse the command line
    char* argv[MAX_ARGS];
    int argc = parse_command(cmd_copy, argv);
    
    if (argc == 0) return;
    
    // Execute built-in command
    for (int i = 0; builtin_commands[i].name; i++) {
        if (strcmp(argv[0], builtin_commands[i].name) == 0) {
            builtin_commands[i].function(argc, argv);
            return;
        }
    }
    
    shell_print_error("Command not found: %s\n", argv[0]);
}

// ==================== Built-in Commands ====================
void cmd_help(int argc, char* argv[]) {
    (void)argc; (void)argv;
    shell_print_info("\nAvailable commands:\n");
    
    for (int i = 0; builtin_commands[i].name; i++) {
        vga_print("  ");
        vga_print(builtin_commands[i].name);
        // Pad to 10 characters
        int pad = 10 - strlen(builtin_commands[i].name);
        while (pad-- > 0) vga_print(" ");
        vga_print(" - ");
        vga_print(builtin_commands[i].description);
        vga_print("\n");
    }
    vga_print("\n");
}

void cmd_echo(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        vga_print(argv[i]);
        if (i < argc - 1) {
            vga_print(" ");
        }
    }
    vga_print("\n");
}

void cmd_clear(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_clear();
}

void cmd_ls(int argc, char* argv[]) {
    (void)argc; (void)argv; // Mark as unused for now
    
    // For now, just show a simple message
    vga_print("Directory listing not implemented\n");
    
    // TODO: Implement actual directory listing with options
    // const char* path = ".";
    // int show_all = 0;
    // int long_format = 0;
    // ... rest of the implementation
}

void cmd_cat(int argc, char* argv[]) {
    if (argc < 2) {
        shell_print_error("Usage: cat <file>\n");
        return;
    }
    
    // TODO: Implement file reading
    shell_print_info("Reading file: %s\n", argv[1]);
}

void cmd_mkdir(int argc, char* argv[]) {
    if (argc < 2) {
        shell_print_error("Usage: mkdir <directory>\n");
        return;
    }
    
    // TODO: Implement directory creation
    shell_print_success("Created directory: %s\n", argv[1]);
}

void cmd_rm(int argc, char* argv[]) {
    if (argc < 2) {
        shell_print_error("Usage: rm <file>\n");
        return;
    }
    
    // TODO: Implement file/directory removal
    shell_print_info("Removed: %s\n", argv[1]);
}

void cmd_cd(int argc, char* argv[]) {
    const char* path = "/"; // Default to root
    
    if (argc > 1) {
        path = argv[1];
    } else {
        // If no argument, try to get home directory
        // In a real OS, we would use getenv("HOME") or similar
        path = "/";
    }
    
    // Simple path handling for now
    if (strcmp(path, "..") == 0) {
        // Go up one directory
        char* last_slash = strrchr(shell_state.cwd, '/');
        if (last_slash && last_slash != shell_state.cwd) {
            *last_slash = '\0';
        } else if (last_slash == shell_state.cwd) {
            // At root
            shell_state.cwd[1] = '\0';
        }
    } else if (path[0] == '/') {
        // Absolute path
        strncpy(shell_state.cwd, path, MAX_PATH_LENGTH - 1);
        shell_state.cwd[MAX_PATH_LENGTH - 1] = '\0';
    } else {
        // Relative path
        size_t len = strlen(shell_state.cwd);
        if (len + strlen(path) + 2 < MAX_PATH_LENGTH) {
            if (len > 0 && shell_state.cwd[len-1] != '/') {
                strcat(shell_state.cwd, "/");
            }
            strncat(shell_state.cwd, path, MAX_PATH_LENGTH - strlen(shell_state.cwd) - 1);
        }
    }
}

void cmd_pwd(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_print(shell_state.cwd);
    vga_print("\n");
}

void cmd_ps(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_print("  PID CMD\n");
    // TODO: Implement process listing
    vga_print("    1 shell\n");
}

void cmd_kill(int argc, char* argv[]) {
    if (argc < 2) {
        shell_print_error("Usage: kill <pid>\n");
        return;
    }
    
    int pid = atoi(argv[1]);
    // TODO: Implement process termination
    shell_print_success("Killed process %d\n", pid);
}

void cmd_ping(int argc, char* argv[]) {
    if (argc < 2) {
        shell_print_error("Usage: ping <host>\n");
        return;
    }
    
    // TODO: Implement ping
    shell_print_info("PING %s: Not implemented\n", argv[1]);
}

void cmd_ifconfig(int argc, char* argv[]) {
    (void)argc; (void)argv;
    vga_print("lo0: flags=8049<UP,LOOPBACK,RUNNING,MULTICAST> mtu 16384\n");
    vga_print("        inet 127.0.0.1 netmask 0xff000000\n");
    vga_print("        inet6 ::1 prefixlen 128\n");
}

void cmd_alias(int argc, char* argv[]) {
    if (argc == 1) {
        // List all aliases
        if (shell_state.alias_count == 0) {
            vga_print("No aliases defined.\n");
            return;
        }
        
        for (int i = 0; i < shell_state.alias_count; i++) {
            vga_print("alias ");
            vga_print(shell_state.aliases[i].name);
            vga_print("='");
            vga_print(shell_state.aliases[i].value);
            vga_print("'\n");
        }
        return;
    }
    
    // Set alias
    if (argc == 2) {
        shell_print_error("Usage: alias name='value'\n");
        return;
    }
    
    // Extract name and value from "name=value"
    char* equal_sign = strchr(argv[1], '=');
    if (!equal_sign) {
        shell_print_error("Invalid alias format. Use: alias name='value'\n");
        return;
    }
    
    *equal_sign = '\0';
    const char* name = argv[1];
    const char* value = equal_sign + 1;
    
    // Remove quotes if present
    if (*value == '"' || *value == '\'') {
        char quote = *value;
        value++;
        char* end_quote = strrchr(value, quote);
        if (end_quote) *end_quote = '\0';
    }
    
    if (shell_add_alias(name, value) == 0) {
        vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
        vga_print("Success: Alias set: ");
        vga_print(name);
        vga_print("='");
        vga_print(value);
        vga_print("'\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    } else {
        vga_set_color(VGA_COLOR_RED, VGA_COLOR_BLACK);
        vga_print("Error: Failed to set alias. Maximum number of aliases reached.\n");
        vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    }
}

void cmd_unalias(int argc, char* argv[]) {
    if (argc < 2) {
        shell_print_error("Usage: unalias <name>\n");
        return;
    }
    
    const char* name = argv[1];
    for (int i = 0; i < shell_state.alias_count; i++) {
        if (strcmp(shell_state.aliases[i].name, name) == 0) {
            // Remove alias by shifting remaining aliases
            for (int j = i; j < shell_state.alias_count - 1; j++) {
                strcpy(shell_state.aliases[j].name, shell_state.aliases[j+1].name);
                strcpy(shell_state.aliases[j].value, shell_state.aliases[j+1].value);
            }
            shell_state.alias_count--;
            shell_print_success("Removed alias: %s\n", name);
            return;
        }
    }
    
    shell_print_error("Alias not found: %s\n", name);
}

void cmd_history(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    if (history.count == 0) {
        vga_print("No command history.\n");
        return;
    }
    
    for (int i = 0; i < history.count; i++) {
        // Print line number (5 digits)
        int num = i + 1;
        if (num < 10000) vga_print(" ");
        if (num < 1000) vga_print(" ");
        if (num < 100) vga_print(" ");
        if (num < 10) vga_print(" ");
        
        // Convert number to string manually
        char num_str[6]; // 5 digits + null terminator
        int pos = 5;
        num_str[pos--] = '\0';
        while (num > 0 && pos >= 0) {
            num_str[pos--] = '0' + (num % 10);
            num /= 10;
        }
        while (pos >= 0) {
            num_str[pos--] = ' ';
        }
        vga_print(num_str);
        vga_print("  ");
        vga_print(history.commands[i]);
        vga_print("\n");
    }
}

void cmd_export(int argc, char* argv[]) {
    if (argc < 2) {
        // TODO: List all environment variables
        vga_print("No environment variables set.\n");
        return;
    }
    
    // Parse VAR=value
    char* equal_sign = strchr(argv[1], '=');
    if (!equal_sign) {
        shell_print_error("Invalid format. Use: export VAR=value\n");
        return;
    }
    
    *equal_sign = '\0';
    const char* var = argv[1];
    const char* value = equal_sign + 1;
    
    // TODO: Implement environment variable setting
    vga_set_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK);
    vga_print("Success: Exported ");
    vga_print(var);
    vga_print("=");
    vga_print(value);
    vga_print("\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
}
