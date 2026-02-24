CC=gcc
CFLAGS=-Wall -Wextra -g

all: myshell

myshell: main.c
	$(CC) $(CFLAGS) -o myshell main.c

clean:
	rm -f myshell
