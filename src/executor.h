#ifndef EXECUTOR_H
#define EXECUTOR_H

int shell_execute(char **args);
int shell_execute_line(char **args);

extern int last_command_status;

#endif
