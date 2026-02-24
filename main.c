#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/*
  Function Declarations for builtin shell commands:
 */
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit
};

int shell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
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

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int shell_help(char **args)
{
  int i;
  printf("Basic Unix Shell\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < shell_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int shell_exit(char **args)
{
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
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

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
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

  for (i = 0; i < shell_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return shell_launch(args, run_bg);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *shell_read_line(void)
{
  char *line = NULL;
  size_t bufsize = 0; // have getline allocate a buffer for us
  ssize_t chars_read;

  chars_read = getline(&line, &bufsize, stdin);

  if (chars_read == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **shell_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void shell_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    // Reap zombies before prompting
    while (waitpid(-1, NULL, WNOHANG) > 0);

    printf("myshell> ");
    line = shell_read_line();
    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  shell_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
