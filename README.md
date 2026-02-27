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

## Advanced Usage

### Personalization (`.myshellrc`)
The shell runs `~/.myshellrc` on startup if the file exists. You can use it to automatically set aliases or environment variables. 
**Important Note:** When creating `.myshellrc` from your host Linux/macOS/WSL Bash terminal, be careful with exclamation marks (`!`) inside double quotes, as Bash will interpret them as history expansion. Use single quotes for the outer string:
```bash
echo 'alias hello="echo Hello from myshellrc!"' >> ~/.myshellrc
```
Once inside `myshell`, the prompt dynamically updates to show your current working directory with color coding:
`myshell: /current/dir$ `

### Productivity & Aliasing
- **Aliases:** Set custom command shortcuts.
  ```bash
  myshell: /current/dir$ alias ll='ls -l'
  myshell: /current/dir$ ll
  myshell: /current/dir$ alias
  alias ll='ls -l'
  myshell: /current/dir$ unalias ll
  ```
- **Directory Stack:** Quickly save and navigate between directories.
  ```bash
  myshell: /current/dir$ pushd /tmp
  myshell: /tmp$ pushd /usr
  myshell: /usr$ dirs
  /usr /tmp /current/dir
  myshell: /usr$ popd
  myshell: /tmp$
  ```

### Advanced Stream & I/O
The shell handles standard Unix streams, empowering complex pipelines.
- **Append Redirection:** Append output to a file instead of overwriting it (`>>`).
  ```bash
  myshell: /tmp$ echo "First line" > log.txt
  myshell: /tmp$ echo "Second line" >> log.txt
  ```
- **Error Redirection:** Redirect standard error (`STDERR`) separately from standard output (`2>`).
  ```bash
  myshell: /tmp$ ls nonexistent 2> error_log.txt
  ```
- **Combined Redirection:** Redirect both standard output and standard error to the same file (`&>`).
  ```bash
  myshell: /tmp$ make &> build.log
  ```

### POSIX Job Control
You can manage processes directly from the shell using advanced job control mechanics exactly like Bash or Zsh.
- **Background Execution:** Run a command without blocking the prompt by appending `&`.
  ```bash
  myshell: /tmp$ sleep 10 &
  [1] 12345
  ```
- **Process Suspension:** Press `Ctrl+Z` to suspend (stop) the currently running foreground process and push it to the background.
  ```bash
  myshell: /tmp$ sleep 20
  ^Z
  [2]+  Stopped                 sleep 20
  ```
- **Job Management:** Track and manipulate jobs using built-in commands:
  - `jobs`: List all active running or stopped jobs.
  - `fg [job_id]`: Bring a background or stopped job to the foreground.
  - `bg [job_id]`: Resume a suspended job in the background.

## Roadmap / Planned Features
- [x] **Usability & Quality of Life:** 
  - [x] Add Signal Handling (Ctrl+C).
  - [x] Support Command History (Up/Down Arrows) with GNU `readline`.
  - [x] Support Tab Auto-completion.
- [x] **Advanced Variable & Logic Handling:** 
  - [x] Parse Environment Variables (`export`, `$VAR`).
  - [x] Parse Logical Operators (`&&`, `||`).
  - [x] Add Globbing (Wildcards `*`, `?`).
- [x] **Personalization & Persistence:**
  - [x] Support Startup Scripts (`.myshellrc`).
  - [x] Implement Dynamic Prompt Engine (ANSI colors, `getcwd()`).
- [x] **Productivity & Aliasing:**
  - [x] Command Aliases (`alias`, `unalias`).
  - [x] Directory Stack (`pushd`, `popd`, `dirs`).
- [x] **Advanced Stream & I/O Control:**
  - [x] Append Redirection (`>>`).
  - [x] Standard Error Handling (`2>`, `&>`).
- [x] **POSIX Job Control (Advanced):**
  - [x] Implement Job Tracking (`jobs`).
  - [x] Add Process Suspension (Ctrl+Z).
  - [x] Foreground & Background Management (`fg`, `bg`).

---
*Built as an educational project exploring Unix system architecture.*
