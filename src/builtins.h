#ifndef BUILTINS_H
#define BUILTINS_H

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_export(char **args);
int shell_num_builtins(void);
int execute_builtin(char **args);

extern char *builtin_str[];

#endif
