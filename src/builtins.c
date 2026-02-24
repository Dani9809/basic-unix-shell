#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtins.h"

char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "export"
};

int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit,
  &shell_export
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
