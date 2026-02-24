#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include "shell.h"
#include "parser.h"
#include "executor.h"

void shell_loop(void)
{
  char *line;
  char **args;
  int status;

  shell_init_readline();

  // Ignore SIGINT in the shell's main loop.
  // Child processes should restore it to default.
  signal(SIGINT, SIG_IGN);

  do {
    // Reap zombies before prompting
    while (waitpid(-1, NULL, WNOHANG) > 0);

    line = shell_read_line("myshell> ");
    if (!line) {
        // EOF or error
        break;
    }
    
    args = shell_split_line(line);
    
    if (args && args[0]) {
        char **expanded_args = shell_expand_args(args);
        
        // Backup pointers to free properly
        int count = 0;
        while(expanded_args[count]) count++;
        char **to_free = malloc((count + 1) * sizeof(char*));
        for (int i = 0; i < count; i++) to_free[i] = expanded_args[i];
        
        status = shell_execute_line(expanded_args);
        
        for (int i = 0; i < count; i++) free(to_free[i]);
        free(to_free);
        free(expanded_args);
    } else {
        status = 1; // Empty line
    }

    free(line);
    free(args);
  } while (status);
}
