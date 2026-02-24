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

## Compilation and Execution

Since this is a Unix-based shell utilizing strict POSIX system calls (`<sys/wait.h>`), it must be compiled in a Unix-like environment (Linux, macOS, WSL, or Cygwin). It *cannot* be compiled natively using Windows MSVC.

### Prerequisites
- GCC Compiler (`build-essential`)
- Make

### Build Steps

1. Clone the repository:
```bash
git clone <your-repo-url>
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
myshell> help
myshell> exit
```

## Roadmap / Planned Features
- [ ] Output redirection (`>`)
- [ ] Input redirection (`<`)
- [ ] Command piping (`|`)
- [ ] Background processes (`&`)

---
*Built as an educational project exploring Unix system architecture.*
