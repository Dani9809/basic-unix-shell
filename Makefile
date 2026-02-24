CC=gcc
CFLAGS=-Wall -Wextra -g

SRCS = src/main.c src/shell.c src/parser.c src/executor.c src/builtins.c
OBJS = $(SRCS:.c=.o)

all: myshell

myshell: $(OBJS)
	$(CC) $(CFLAGS) -o myshell $(OBJS)

clean:
	rm -f myshell src/*.o
