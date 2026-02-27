#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shell.h"

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  // Run .myshellrc if it exists
  char *home = getenv("HOME");
  if (home) {
      char *rc_path = malloc(strlen(home) + 12);
      sprintf(rc_path, "%s/.myshellrc", home);
      shell_run_file(rc_path);
      free(rc_path);
  }

  // Run command loop.
  shell_loop();

  return EXIT_SUCCESS;
}
