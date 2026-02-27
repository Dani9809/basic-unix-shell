#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "executor.h"
#include "builtins.h"

int last_command_status = 0;
pid_t shell_pgid = 0;
int shell_terminal = STDIN_FILENO;

void setup_redirection(char **args) {
  char *input_file = NULL;
  char *output_file = NULL;
  char *err_file = NULL;
  int append = 0;
  int redirect_err = 0;
  int cmd_end = -1;
  
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "<") == 0) {
      if (cmd_end == -1) cmd_end = i;
      input_file = args[i+1];
    } else if (strcmp(args[i], ">") == 0) {
      if (cmd_end == -1) cmd_end = i;
      output_file = args[i+1];
    } else if (strcmp(args[i], ">>") == 0) {
      if (cmd_end == -1) cmd_end = i;
      output_file = args[i+1];
      append = 1;
    } else if (strcmp(args[i], "2>") == 0) {
      if (cmd_end == -1) cmd_end = i;
      err_file = args[i+1];
    } else if (strcmp(args[i], "&>") == 0) {
      if (cmd_end == -1) cmd_end = i;
      output_file = args[i+1];
      redirect_err = 1;
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
    int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
    int fd_out = open(output_file, flags, 0644);
    if (fd_out < 0) { perror("myshell: output file"); exit(EXIT_FAILURE); }
    dup2(fd_out, STDOUT_FILENO);
    if (redirect_err) {
        dup2(fd_out, STDERR_FILENO);
    }
    close(fd_out);
  }
  
  if (err_file && !redirect_err) {
      int fd_err = open(err_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd_err < 0) { perror("myshell: err file"); exit(EXIT_FAILURE); }
      dup2(fd_err, STDERR_FILENO);
      close(fd_err);
  }
}

int shell_launch(char **args, int run_bg)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    pid_t cpid = getpid();
    setpgid(cpid, cpid);
    if (!run_bg) {
        tcsetpgrp(shell_terminal, cpid);
    }
    
    // Restore default signal handlers for the child
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
    signal(SIGTTOU, SIG_DFL);
    
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
    setpgid(pid, pid); // Prevent race condition
    
    // Copy the command for display
    char cmd[1024] = {0};
    for(int j=0; args[j]!=NULL; j++) {
        strcat(cmd, args[j]);
        if(args[j+1]!=NULL) strcat(cmd, " ");
    }
    
    if (!run_bg) {
      tcsetpgrp(shell_terminal, pid);
      add_job(pid, 0, cmd);
      struct Job *j = find_job_by_pid(pid);
      if (j) wait_for_job(j);
    } else {
      add_job(pid, 1, cmd);
      struct Job *j = find_job_by_pid(pid);
      if (j) printf("[%d] %d\n", j->id, pid);
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
    last_command_status = 0;
    return builtin_res;
  }

  return shell_launch(args, run_bg);
}

int shell_execute_line(char **args) {
    if (args[0] == NULL) return 1;

    int i = 0;
    int start = 0;
    int loop_status = 1;
    int skip_next = 0;
    
    while (args[i] != NULL) {
        if (strcmp(args[i], "&&") == 0 || strcmp(args[i], "||") == 0) {
            char *op = args[i];
            args[i] = NULL;
            
            if (!skip_next && args[start] != NULL) {
                loop_status = shell_execute(&args[start]);
                if (loop_status == 0) return 0; // Exit shell
            }
            
            if (strcmp(op, "&&") == 0) {
                skip_next = (last_command_status != 0);
            } else if (strcmp(op, "||") == 0) {
                skip_next = (last_command_status == 0);
            }
            
            start = i + 1;
        }
        i++;
    }
    
    if (args[start] != NULL && !skip_next) {
        loop_status = shell_execute(&args[start]);
    }
    
    return loop_status;
}
