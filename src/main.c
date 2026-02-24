#include <stdlib.h>
#include "shell.h"

int main(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  // Run command loop.
  shell_loop();

  return EXIT_SUCCESS;
}
