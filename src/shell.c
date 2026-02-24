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

  // Ignore SIGINT in the shell's main loop.
  // Child processes should restore it to default.
  signal(SIGINT, SIG_IGN);

  do {
    // Reap zombies before prompting
    while (waitpid(-1, NULL, WNOHANG) > 0);

    printf("myshell> ");
    fflush(stdout);

    line = shell_read_line();
    if (!line) {
        // EOF or error
        break;
    }
    
    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
}
