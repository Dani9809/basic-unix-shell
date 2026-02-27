#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <sys/types.h>

int shell_execute(char **args);
int shell_execute_line(char **args);

extern int last_command_status;
extern pid_t shell_pgid;
extern int shell_terminal;

#endif

