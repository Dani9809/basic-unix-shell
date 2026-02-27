#ifndef BUILTINS_H
#define BUILTINS_H

#include <sys/types.h>

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_export(char **args);
int shell_alias(char **args);
int shell_unalias(char **args);
int shell_pushd(char **args);
int shell_popd(char **args);
int shell_dirs(char **args);
int shell_jobs(char **args);
int shell_fg(char **args);
int shell_bg(char **args);
int shell_num_builtins(void);
int execute_builtin(char **args);
char *resolve_alias(const char *name);

typedef enum {
    JOB_FOREGROUND,
    JOB_RUNNING,
    JOB_STOPPED
} JobState;

struct Job {
    int id;
    pid_t pid;
    char *cmd;
    JobState state;
    struct Job *next;
};

extern struct Job *first_job;

void add_job(pid_t pid, int bg, const char *cmd);
void remove_job(pid_t pid);
struct Job *find_job_by_pid(pid_t pid);
void wait_for_job(struct Job *job);

extern char *builtin_str[];

#endif


