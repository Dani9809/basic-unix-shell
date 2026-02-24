# Basic Unix Shell in C 🐚

A lightweight, functional Unix shell built from scratch in C. 

This project explores the core concepts behind how an operating system's command-line interface works, focusing on process creation, memory management, and system calls.

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
  - [ ] Add Signal Handling (Ctrl+C).
  - [ ] Support Command History (Up/Down Arrows) with GNU `readline`.
  - [ ] Support Tab Auto-completion.
- [ ] **Advanced Variable & Logic Handling:** 
  - [ ] Parse Environment Variables (`export`, `$VAR`).
  - [ ] Parse Logical Operators (`&&`, `||`).
  - [ ] Add Globbing (Wildcards `*`, `?`).

---
*Built as an educational project exploring Unix system architecture.*
