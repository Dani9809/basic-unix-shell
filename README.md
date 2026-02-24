# Basic Unix Shell in C 🐚

A lightweight, functional Unix shell built from scratch in C. 

This project explores the core concepts behind how an operating system's command-line interface works, focusing on process creation, memory management, and system calls. Recently, the architecture was refactored into a modular design to improve maintainability, scalability, and security.

## System Architecture

The shell is organized into discrete modules, each handling a specific part of the command execution lifecycle:

```text
+----------------------------------------------------+
|                      main.c                        |
|  (Entry point, initializes and starts the shell)   |
+-------------------------+--------------------------+
                          |
                          v
+----------------------------------------------------+
|                     shell.c                        |
| (Main REPL loop: Read, Evaluate, Print, Loop and   |
|               SIGINT handling)                     |
+-------------------------+--------------------------+
                          |
             +------------+-------------+
             |                          |
             v                          v
+-------------------------+  +-----------------------+
|        parser.c         |  |      executor.c       |
| (Reads input line and   |  | (Handles process      |
| tokenizes into args)    |  | spawning, pipes, I/O  |
|                         |  | redirection, etc.)    |
+-------------------------+  +----------+------------+
                                        |
                                        v
                             +-----------------------+
                             |      builtins.c       |
                             | (cd, help, exit)      |
                             +-----------------------+
```

### Modules Directory (`src/`)
- `main.c`: The executable entry point. Responsible for initial setup before launching the interactive loop.
- `shell.c`: Contains the core infinite REPL loop and signal handling (e.g., ignoring `SIGINT` so Ctrl+C doesn't kill the shell framework).
- `parser.c`: Handles dynamic memory allocation to read arbitrary-length input safely and tokenizes commands.
- `executor.c`: The core operating system interface. Sets up pipes, parses I/O redirection (`>`, `<`), and manages background tasks before invoking `fork()` and `execvp()`.
- `builtins.c`: Built-in shell commands that must be executed directly by the parent shell process (such as changing directories or exiting).

## Features Currently Implemented
- **REPL Loop:** Continuously Reads, Evaluates, Prints, and Loops user input.
- **Dynamic Parsing:** Efficiently tokenizes string inputs mapping to commands and arguments.
- **Process Spawning:** Executes external programs using the POSIX `fork()`, `execvp()`, and `waitpid()` system calls.
- **Built-in Commands:**
  - `cd`: Changes the current working directory.
  - `help`: Displays a list of built-in commands.
  - `exit`: Safely terminates the shell loop.
- **Advanced Features:**
  - **I/O Redirection:** Enables reading from or writing output directly to files (`<`, `>`).
  - **Piping:** Connects multiple commands seamlessly sending the output of one process as standard input for another (`|`).
  - **Background Processes:** Run non-blocking shell tasks by appending an ampersand (`&`).
- **Resilience:** Built-in `SIGINT` (Ctrl+C) handling prevents accidental termination of the shell environment.

## Compilation and Execution

Since this is a Unix-based shell utilizing strict POSIX system calls (`<sys/wait.h>`), it must be compiled in a Unix-like environment (Linux, macOS, WSL, or Cygwin). It *cannot* be compiled natively using Windows MSVC.

### Prerequisites
- GCC Compiler (`build-essential`)
- Make

### Build Steps

1. Clone the repository:
```bash
git clone https://github.com/Dani9809/basic-unix-shell.git
cd basic-unix-shell
```

2. Compile the shell using `make`:
```bash
make
```

3. Run the executable:
```bash
./myshell
```

## Usage Example

```bash
myshell> ls -l
myshell> pwd
myshell> echo "Hello World!"
myshell> echo "Piping test" | grep test
myshell> sleep 5 &
myshell> help
myshell> exit
```

## Roadmap / Planned Features
- [ ] **Usability & Quality of Life:** 
  - [x] Add Signal Handling (Ctrl+C).
  - [x] Support Command History (Up/Down Arrows) with GNU `readline`.
  - [x] Support Tab Auto-completion.
- [ ] **Advanced Variable & Logic Handling:** 
  - [ ] Parse Environment Variables (`export`, `$VAR`).
  - [ ] Parse Logical Operators (`&&`, `||`).
  - [ ] Add Globbing (Wildcards `*`, `?`).

---
*Built as an educational project exploring Unix system architecture.*
