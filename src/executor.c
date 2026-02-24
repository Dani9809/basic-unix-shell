#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "executor.h"
#include "builtins.h"

void setup_redirection(char **args) {
  char *input_file = NULL;
  char *output_file = NULL;
  int cmd_end = -1;
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "<") == 0) {
      if (cmd_end == -1) cmd_end = i;
      input_file = args[i+1];
    } else if (strcmp(args[i], ">") == 0) {
      if (cmd_end == -1) cmd_end = i;
      output_file = args[i+1];
    }
  }
  if (cmd_end != -1) {
    args[cmd_end] = NULL;
  }

  if (input_file) {
    int fd_in = open(input_file, O_RDONLY);
    if (fd_in < 0) { perror("myshell: input file"); exit(EXIT_FAILURE); }
    dup2(fd_in, STDIN_FILENO);
    close(fd_in);
  }
  if (output_file) {
    int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) { perror("myshell: output file"); exit(EXIT_FAILURE); }
    dup2(fd_out, STDOUT_FILENO);
    close(fd_out);
  }
}

int shell_launch(char **args, int run_bg)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    // Restore default SIGINT behavior for the child
    signal(SIGINT, SIG_DFL);
    
    setup_redirection(args);
    if (execvp(args[0], args) == -1) {
      perror("myshell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("myshell");
  } else {
    // Parent process
    if (!run_bg) {
      do {
        wpid = waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    } else {
      printf("[Started background process %d]\n", pid);
    }
  }

  return 1;
}

int shell_execute(char **args)
{
  int i;
  int run_bg = 0;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  // Check for background process
  int last_idx = 0;
  while (args[last_idx] != NULL) last_idx++;
  if (last_idx > 0 && strcmp(args[last_idx-1], "&") == 0) {
    run_bg = 1;
    args[last_idx-1] = NULL;
  }

  // Check for pipe
  int pipe_idx = -1;
  for (i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      pipe_idx = i;
      break;
    }
  }

  if (pipe_idx != -1) {
    args[pipe_idx] = NULL;
    char **left_args = args;
    char **right_args = &args[pipe_idx + 1];

    int pipefd[2];
    if (pipe(pipefd) < 0) {
      perror("myshell: pipe");
      return 1;
    }

    pid_t p1 = fork();
    if (p1 == 0) {
      // Child 1 (Left)
      signal(SIGINT, SIG_DFL);
      dup2(pipefd[1], STDOUT_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
      setup_redirection(left_args);
      if (execvp(left_args[0], left_args) == -1) {
        perror("myshell");
      }
      exit(EXIT_FAILURE);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
      // Child 2 (Right)
      signal(SIGINT, SIG_DFL);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
      close(pipefd[1]);
      setup_redirection(right_args);
      if (execvp(right_args[0], right_args) == -1) {
        perror("myshell");
      }
      exit(EXIT_FAILURE);
    }

    close(pipefd[0]);
    close(pipefd[1]);

    if (!run_bg) {
      waitpid(p1, NULL, 0);
      waitpid(p2, NULL, 0);
    } else {
      printf("[Started background processes %d, %d]\n", p1, p2);
    }
    return 1;
  }

  int builtin_res = execute_builtin(args);
  if (builtin_res != -1) {
    return builtin_res;
  }

  return shell_launch(args, run_bg);
}
