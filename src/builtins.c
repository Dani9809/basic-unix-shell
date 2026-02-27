#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtins.h"

char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "export",
  "alias",
  "unalias",
  "pushd",
  "popd",
  "dirs",
  "jobs",
  "fg",
  "bg"
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit,
  &shell_export,
  &shell_alias,
  &shell_unalias,
  &shell_pushd,
  &shell_popd,
  &shell_dirs,
  &shell_jobs,
  &shell_fg,
  &shell_bg
};

int shell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int execute_builtin(char **args) {
  for (int i = 0; i < shell_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return -1; // Not a builtin
}

int shell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "myshell: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("myshell");
    }
  }
  return 1;
}

int shell_help(char **args)
{
  (void)args; // unused
  printf("Basic Unix Shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following commands are built-in:\n");
  printf("  cd <dir>  - Change the current working directory.\n");
  printf("  help      - Print this help information.\n");
  printf("  exit      - Safely terminate the shell.\n");
  
  printf("\nSupported Shell Features:\n");
  printf("  <         - Redirect input from a file.\n");
  printf("  >         - Redirect output to a file.\n");
  printf("  |         - Pipe the output of one command to another.\n");
  printf("  &         - Run the command in the background.\n");
  printf("  Up/Down   - Cycle through command history.\n");
  
  printf("\nMost standard Unix commands (e.g., ls, pwd, echo, cat) form external processes.\n");
  printf("Use the man command for detailed information on other programs.\n");
  return 1;
}

int shell_exit(char **args)
{
  (void)args; // unused
  return 0;
}

int shell_export(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "myshell: expected argument to \"export\", e.g., export VAR=value\n");
    return 1;
  }
  
  // Find the '=' character
  char *eq_pos = strchr(args[1], '=');
  if (eq_pos == NULL) {
    fprintf(stderr, "myshell: invalid format for export, use VAR=value\n");
    return 1;
  }
  
  // Split name and value securely
  *eq_pos = '\0';
  char *name = args[1];
  char *value = eq_pos + 1;
  
  if (setenv(name, value, 1) != 0) {
    perror("myshell: export");
  }
  
  return 1;
}

struct Alias {
    char *name;
    char *value;
    struct Alias *next;
};
struct Alias *alias_head = NULL;

int shell_alias(char **args) {
    if (args[1] == NULL) {
        struct Alias *curr = alias_head;
        while (curr) {
            printf("alias %s='%s'\n", curr->name, curr->value);
            curr = curr->next;
        }
        return 1;
    }
    
    char buf[1024] = {0};
    for (int i = 1; args[i] != NULL; i++) {
        strcat(buf, args[i]);
        if (args[i+1] != NULL) strcat(buf, " ");
    }
    
    char *eq_pos = strchr(buf, '=');
    if (!eq_pos) {
        struct Alias *curr = alias_head;
        while (curr) {
            if (strcmp(curr->name, buf) == 0) {
                printf("alias %s='%s'\n", curr->name, curr->value);
                return 1;
            }
            curr = curr->next;
        }
        fprintf(stderr, "myshell: alias: %s: not found\n", buf);
        return 1;
    }
    
    *eq_pos = '\0';
    char *name = buf;
    char *value = eq_pos + 1;
    
    int len = strlen(value);
    if (len >= 2 && ((value[0] == '\'' && value[len-1] == '\'') || (value[0] == '"' && value[len-1] == '"'))) {
        value[len-1] = '\0';
        value++;
    }
    
    struct Alias *curr = alias_head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            free(curr->value);
            curr->value = strdup(value);
            return 1;
        }
        curr = curr->next;
    }
    
    struct Alias *new_alias = malloc(sizeof(struct Alias));
    new_alias->name = strdup(name);
    new_alias->value = strdup(value);
    new_alias->next = alias_head;
    alias_head = new_alias;
    
    return 1;
}

int shell_unalias(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "unalias: usage: unalias name\n");
        return 1;
    }
    
    struct Alias *curr = alias_head;
    struct Alias *prev = NULL;
    while (curr) {
        if (strcmp(curr->name, args[1]) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                alias_head = curr->next;
            }
            free(curr->name);
            free(curr->value);
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    fprintf(stderr, "myshell: unalias: %s: not found\n", args[1]);
    return 1;
}

char *resolve_alias(const char *name) {
    struct Alias *curr = alias_head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return curr->value;
        }
        curr = curr->next;
    }
    return NULL;
}

#define DIR_STACK_SIZE 128
char *dir_stack[DIR_STACK_SIZE];
int dir_stack_top = 0;

int shell_dirs(char **args) {
    (void)args;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s ", cwd);
    }
    for (int i = dir_stack_top - 1; i >= 0; i--) {
        printf("%s ", dir_stack[i]);
    }
    printf("\n");
    return 1;
}

int shell_pushd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "myshell: pushd: no other directory\n");
        return 1;
    }
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("myshell: pushd");
        return 1;
    }
    
    if (chdir(args[1]) != 0) {
        perror("myshell: pushd");
    } else {
        if (dir_stack_top < DIR_STACK_SIZE) {
            dir_stack[dir_stack_top++] = strdup(cwd);
            shell_dirs(NULL);
        } else {
            fprintf(stderr, "myshell: pushd: directory stack full\n");
        }
    }
    return 1;
}

int shell_popd(char **args) {
    (void)args;
    if (dir_stack_top > 0) {
        dir_stack_top--;
        char *target = dir_stack[dir_stack_top];
        if (chdir(target) != 0) {
            perror("myshell: popd");
        } else {
            shell_dirs(NULL);
        }
        free(target);
    } else {
        fprintf(stderr, "myshell: popd: directory stack empty\n");
    }
    return 1;
}

struct Job *first_job = NULL;
int next_job_id = 1;

void add_job(pid_t pid, int bg, const char *cmd) {
    struct Job *new_job = malloc(sizeof(struct Job));
    new_job->id = next_job_id++;
    new_job->pid = pid;
    new_job->cmd = strdup(cmd);
    new_job->state = bg ? JOB_RUNNING : JOB_FOREGROUND;
    new_job->next = NULL;
    
    if (first_job == NULL) {
        first_job = new_job;
    } else {
        struct Job *curr = first_job;
        while (curr->next != NULL) curr = curr->next;
        curr->next = new_job;
    }
}

void remove_job(pid_t pid) {
    struct Job *curr = first_job, *prev = NULL;
    while (curr != NULL) {
        if (curr->pid == pid) {
            if (prev == NULL) first_job = curr->next;
            else prev->next = curr->next;
            free(curr->cmd);
            free(curr);
            if (first_job == NULL) next_job_id = 1;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

struct Job *find_job_by_pid(pid_t pid) {
    struct Job *curr = first_job;
    while (curr) {
        if (curr->pid == pid) return curr;
        curr = curr->next;
    }
    return NULL;
}

int shell_jobs(char **args) {
    (void)args;
    struct Job *curr = first_job;
    while (curr) {
        if (curr->state != JOB_FOREGROUND) {
            printf("[%d] %s    %s\n", curr->id, 
                curr->state == JOB_STOPPED ? "Stopped" : "Running", curr->cmd);
        }
        curr = curr->next;
    }
    return 1;
}

#include <sys/wait.h>
extern pid_t shell_pgid;
extern int shell_terminal;

void wait_for_job(struct Job *job) {
    int status;
    pid_t pid = job->pid;
    
    tcsetpgrp(shell_terminal, pid);
    
    if (waitpid(pid, &status, WUNTRACED) > 0) {
        if (WIFSTOPPED(status)) {
            job->state = JOB_STOPPED;
            printf("\n[%d]+  Stopped                 %s\n", job->id, job->cmd);
        } else {
            remove_job(pid);
        }
    }
    
    tcsetpgrp(shell_terminal, shell_pgid);
}

int shell_fg(char **args) {
    struct Job *job = NULL;
    if (args[1] == NULL) {
        // Find last job
        struct Job *curr = first_job;
        while (curr) {
            if (curr->state == JOB_RUNNING || curr->state == JOB_STOPPED) job = curr;
            curr = curr->next;
        }
    } else {
        int target_id = atoi(args[1]);
        struct Job *curr = first_job;
        while (curr) {
            if (curr->id == target_id) { job = curr; break; }
            curr = curr->next;
        }
    }
    
    if (job) {
        printf("%s\n", job->cmd);
        if (job->state == JOB_STOPPED) {
            kill(-job->pid, SIGCONT);
        }
        job->state = JOB_FOREGROUND;
        wait_for_job(job);
    } else {
        fprintf(stderr, "myshell: fg: current: no such job\n");
    }
    return 1;
}

int shell_bg(char **args) {
    struct Job *job = NULL;
    if (args[1] == NULL) {
        // Find last stopped job
        struct Job *curr = first_job;
        while (curr) {
            if (curr->state == JOB_STOPPED) job = curr;
            curr = curr->next;
        }
    } else {
        int target_id = atoi(args[1]);
        struct Job *curr = first_job;
        while (curr) {
            if (curr->id == target_id) { job = curr; break; }
            curr = curr->next;
        }
    }
    
    if (job) {
        if (job->state == JOB_STOPPED) {
            printf("[%d]+ %s &\n", job->id, job->cmd);
            job->state = JOB_RUNNING;
            kill(-job->pid, SIGCONT);
        } else {
            fprintf(stderr, "myshell: bg: job already in background\n");
        }
    } else {
        fprintf(stderr, "myshell: bg: current: no such job\n");
    }
    return 1;
}

