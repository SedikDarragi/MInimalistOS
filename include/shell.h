#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>
#include <stdint.h>

#define SHELL_BUFFER_SIZE 256
#define MAX_ARGS 32
#define MAX_HISTORY 32
#define MAX_ALIASES 16
#define MAX_PATH_LENGTH 256

// ANSI color codes
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Command structure
typedef struct {
    const char* name;               // Command name
    void (*function)(int, char**);  // Command function
    const char* description;        // Help description
    int min_args;                   // Minimum number of arguments (including command)
    int max_args;                   // Maximum number of arguments (0 = no limit)
} command_t;

// Alias structure
typedef struct {
    char name[32];
    char value[SHELL_BUFFER_SIZE];
} alias_t;

// Command history structure
typedef struct {
    char commands[MAX_HISTORY][SHELL_BUFFER_SIZE];
    int count;
    int current;
    int saved_pos;  // For cursor position in history navigation
} command_history_t;

// Shell state structure
typedef struct {
    char cwd[MAX_PATH_LENGTH];      // Current working directory
    char hostname[64];              // System hostname
    char username[32];              // Current username
    alias_t aliases[MAX_ALIASES];   // Command aliases
    int alias_count;                // Number of defined aliases
    int last_status;                // Exit status of last command
} shell_state_t;

// Function declarations
void shell_init(void);
void shell_run(void);
void shell_execute_command(const char* command);
void shell_add_to_history(const char* command);
const char* shell_get_previous_command(void);
const char* shell_get_next_command(void);
void shell_set_prompt(const char* prompt);
void shell_register_command(const command_t* cmd);
int shell_add_alias(const char* name, const char* value);
char* shell_expand_aliases(const char* input);

// Built-in command functions
void cmd_help(int argc, char* argv[]);
void cmd_clear(int argc, char* argv[]);
void cmd_echo(int argc, char* argv[]);
void cmd_ls(int argc, char* argv[]);
void cmd_cat(int argc, char* argv[]);
void cmd_mkdir(int argc, char* argv[]);
void cmd_rm(int argc, char* argv[]);
void cmd_cd(int argc, char* argv[]);
void cmd_pwd(int argc, char* argv[]);
void cmd_ps(int argc, char* argv[]);
void cmd_kill(int argc, char* argv[]);
void cmd_ping(int argc, char* argv[]);
void cmd_ifconfig(int argc, char* argv[]);
void cmd_alias(int argc, char* argv[]);
void cmd_unalias(int argc, char* argv[]);
void cmd_history(int argc, char* argv[]);
void cmd_export(int argc, char* argv[]);

// Utility functions
char* shell_readline(const char* prompt);
char** shell_parse_line(char* line, int* count);
int shell_execute_builtin(int argc, char** argv);
void shell_print_error(const char* format, ...);
void shell_print_info(const char* format, ...);
void shell_print_success(const char* format, ...);

// Global shell state
extern shell_state_t shell_state;

#endif // SHELL_H
