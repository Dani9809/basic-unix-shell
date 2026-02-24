#ifndef PARSER_H
#define PARSER_H

void shell_init_readline(void);
char *shell_read_line(const char *prompt);
char **shell_split_line(char *line);

#endif
