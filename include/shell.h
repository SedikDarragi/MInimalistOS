#ifndef SHELL_H
#define SHELL_H

#include <stddef.h>

#define SHELL_BUFFER_SIZE 256
#define MAX_ARGS 16
#define MAX_HISTORY 16

// Command structure
typedef struct {
    const char* name;
    void (*function)(int argc, char* argv[]);
    const char* description;
} command_t;

// Command history structure
typedef struct {
    char commands[MAX_HISTORY][SHELL_BUFFER_SIZE];
    int count;
    int current;
} command_history_t;

// Function declarations
void shell_init(void);
void shell_run(void);
void shell_execute_command(const char* command);
void shell_add_to_history(const char* command);
const char* shell_get_previous_command(void);
const char* shell_get_next_command(void);

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

#endif // SHELL_H
