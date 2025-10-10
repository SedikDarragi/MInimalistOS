#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// VGA text mode implementation
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

// VGA color constants
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = VGA_COLOR_LIGHT_GREY | (VGA_COLOR_BLACK << 4);

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

// Simple string functions
static size_t my_strlen(const char* str) {
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

static void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

// Command structure
typedef struct {
    const char* name;
    void (*func)(int argc, char** argv);
    const char* help;
} command_t;

// Command implementations
static void cmd_help(int argc, char** argv) {
    (void)argc;  // Unused parameter
    (void)argv;  // Unused parameter
    vga_print("Available commands:\n");
    vga_print("  help    - Show this help message\n");
    vga_print("  echo    - Print arguments\n");
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
    (void)argc;  // Unused parameter
    (void)argv;  // Unused parameter
    vga_clear();
}

// Command table
static const command_t commands[] = {
    {"help", cmd_help, "Show this help message"},
    {"echo", cmd_echo, "Print arguments"},
    {"clear", cmd_clear, "Clear the screen"},
    {NULL, NULL, NULL} // Sentinel
};

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

// Find a command by name
static const command_t* find_command(const char* name) {
    for (const command_t* cmd = commands; cmd->name != NULL; cmd++) {
        if (strcmp(cmd->name, name) == 0) {
            return cmd;
        }
    }
    return NULL;
}

// Simple shell state
static struct {
    char current_dir[256];
    char prompt[64];
} shell_state;

// Shell initialization
void shell_init(void) {
    vga_clear();
    vga_print("MinimalOS Shell v1.0\n");
    vga_print("Type 'help' for a list of commands\n");
    
    // Initialize shell state
    strcpy(shell_state.current_dir, "/");
    strcpy(shell_state.prompt, "minios> ");
}

// Shell main function
void shell_run(void) {
    char input[256];
    char* args[32];
    int argc;
    size_t pos = 0;
    
    while (1) {
        vga_print(shell_state.prompt);
        
        // Read a line of input
        pos = 0;
        while (1) {
            char c = keyboard_getchar();
            
            if (c == '\n' || c == '\r') {
                vga_putchar('\n');
                if (pos > 0) {
                    input[pos] = '\0';
                    
                    // Parse command line
                    (void)my_strlen(input);  // Keep the side effect of the function call
                    argc = 0;
                    char* token = input;
                    while (*token == ' ') token++; // Skip leading spaces
                    
                    while (*token && argc < 31) {
                        args[argc++] = token;
                        while (*token && *token != ' ') token++;
                        if (*token) {
                            *token++ = '\0';
                            while (*token == ' ') token++; // Skip spaces
                        }
                    }
                    args[argc] = NULL;
                    
                    // Execute the command if not empty
                    if (argc > 0) {
                        const command_t* cmd = find_command(args[0]);
                        if (cmd != NULL) {
                            cmd->func(argc, args);
                        } else {
                            vga_print("Command not found: ");
                            vga_print(args[0]);
                            vga_putchar('\n');
                        }
                    }
                }
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
    }
}
