#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include "shell.h"
#include "parser.h"
#include "executor.h"
#include "builtins.h"

void shell_process_line(char *line, int *status_out) {
    char **args = shell_split_line(line);
    
    if (args && args[0]) {
        char *alias_val = resolve_alias(args[0]);
        char **alias_args = NULL;
        char *val_copy = NULL;
        char **base_args = args;
        
        if (alias_val) {
            val_copy = strdup(alias_val);
            alias_args = shell_split_line(val_copy);
            
            int c_alias = 0, c_args = 0;
            while(alias_args[c_alias]) c_alias++;
            while(args[c_args]) c_args++;
            
            char **merged = malloc((c_alias + c_args) * sizeof(char*));
            int p = 0;
            for(int i=0; i<c_alias; i++) merged[p++] = alias_args[i];
            for(int i=1; i<c_args; i++) merged[p++] = args[i];
            merged[p] = NULL;
            base_args = merged;
        }

        char **expanded_args = shell_expand_args(base_args);
        
        if (alias_val) {
            free(base_args);
            free(alias_args);
            free(val_copy);
        }
        
        // Backup pointers to free properly
        int count = 0;
        while(expanded_args[count]) count++;
        char **to_free = malloc((count + 1) * sizeof(char*));
        for (int i = 0; i < count; i++) to_free[i] = expanded_args[i];
        
        *status_out = shell_execute_line(expanded_args);
        
        for (int i = 0; i < count; i++) free(to_free[i]);
        free(to_free);
        free(expanded_args);
    } else {
        *status_out = 1; // Empty line
    }

    free(args);
}

void shell_loop(void)
{
  char *line;
  int status = 1;

  shell_init_readline();

  shell_terminal = STDIN_FILENO;
  shell_pgid = getpid();
  
  if (isatty(shell_terminal)) {
      while (tcgetpgrp(shell_terminal) != shell_pgid)
          kill(-shell_pgid, SIGTTIN);
          
      signal(SIGINT, SIG_IGN);
      signal(SIGQUIT, SIG_IGN);
      signal(SIGTSTP, SIG_IGN);
      signal(SIGTTIN, SIG_IGN);
      signal(SIGTTOU, SIG_IGN);
      
      setpgid(shell_pgid, shell_pgid);
      tcsetpgrp(shell_terminal, shell_pgid);
  }

  do {
    // Reap zombies before prompting
    int wstat;
    pid_t wpid;
    while ((wpid = waitpid(-1, &wstat, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        struct Job *j = find_job_by_pid(wpid);
        if (j) {
            if (WIFEXITED(wstat) || WIFSIGNALED(wstat)) {
                if (j->state == JOB_RUNNING) {
                    printf("\n[%d]+  Done                    %s\n", j->id, j->cmd);
                }
                remove_job(wpid);
            } else if (WIFSTOPPED(wstat)) {
                j->state = JOB_STOPPED;
                printf("\n[%d]+  Stopped                 %s\n", j->id, j->cmd);
            } else if (WIFCONTINUED(wstat)) {
                j->state = JOB_RUNNING;
                printf("\n[%d]+  Continued               %s\n", j->id, j->cmd);
            }
        }
    }

    char prompt[1024];
    char cwd[512];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(prompt, sizeof(prompt), "\033[1;32mmyshell\033[0m:\033[1;34m%s\033[0m$ ", cwd);
    } else {
        snprintf(prompt, sizeof(prompt), "myshell> ");
    }

    line = shell_read_line(prompt);
    if (!line) {
        // EOF or error
        break;
    }
    
    shell_process_line(line, &status);

    free(line);
  } while (status);
}

void shell_run_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    char *line = NULL;
    size_t len = 0;
    int status = 1;
    
    while (getline(&line, &len, fp) != -1 && status) {
        line[strcspn(line, "\r\n")] = 0;
        shell_process_line(line, &status);
    }
    free(line);
    fclose(fp);
}
