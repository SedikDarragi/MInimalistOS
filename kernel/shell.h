#ifndef SHELL_H
#define SHELL_H

#include "kernel.h"

#define SHELL_BUFFER_SIZE 256
#define MAX_ARGS 16

void shell_init(void);
void shell_run(void);
void shell_execute_command(const char* command);

// Built-in commands
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

#endif
