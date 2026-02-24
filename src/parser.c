#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parser.h"

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

#include "builtins.h"

// Generator function for command completion
char *command_generator(const char *text, int state)
{
  static int list_index, len;
  char *name;

  // If this is a new word to complete, initialize now.
  if (!state) {
    list_index = 0;
    len = strlen(text);
  }

  // Return the next name which partially matches from the command list.
  while ((name = builtin_str[list_index])) {
    list_index++;
    if (strncmp(name, text, len) == 0) {
      return strdup(name);
    }
  }

  // If no names matched, then return NULL.
  return NULL;
}

// Custom completion function
char **shell_completion(const char *text, int start, int end)
{
  char **matches = NULL;
  (void)end;

  // If this word is at the start of the line, then it is a command to complete.
  if (start == 0) {
    matches = rl_completion_matches(text, command_generator);
  }

  return matches; // If start > 0, returns NULL, falling back to default filename completion
}

void shell_init_readline(void)
{
  rl_attempted_completion_function = shell_completion;
}

char *shell_read_line(const char *prompt)
{
  char *line = readline(prompt);

  // If EOF is encountered, readline returns NULL.
  if (!line) {
    exit(EXIT_SUCCESS);
  }

  // If line is not empty, add it to history
  if (line[0] != '\0') {
    add_history(line);
  }

  return line;
}

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

#include <glob.h>

char **shell_expand_args(char **args) {
    int bufsize = LSH_TOK_BUFSIZE;
    int position = 0;
    char **new_args = malloc(bufsize * sizeof(char*));

    for (int i = 0; args[i] != NULL; i++) {
        char *token = args[i];
        
        // Variable expansion
        if (token[0] == '$') {
            char *val = getenv(token + 1);
            token = val ? val : "";
        }
        
        // Globbing
        glob_t glob_result;
        memset(&glob_result, 0, sizeof(glob_result));
        int ret = glob(token, GLOB_NOCHECK | GLOB_TILDE, NULL, &glob_result);
        
        if (ret == 0) {
            for (size_t j = 0; j < glob_result.gl_pathc; j++) {
                new_args[position++] = strdup(glob_result.gl_pathv[j]);
                if (position >= bufsize) {
                    bufsize += LSH_TOK_BUFSIZE;
                    new_args = realloc(new_args, bufsize * sizeof(char*));
                }
            }
            globfree(&glob_result);
        } else {
            new_args[position++] = strdup(token);
            if (position >= bufsize) {
                bufsize += LSH_TOK_BUFSIZE;
                new_args = realloc(new_args, bufsize * sizeof(char*));
            }
        }
    }
    new_args[position] = NULL;
    return new_args;
}
