### Basic Unix Shell

## The Core Architecture: REPL
At its heart, a shell is an infinite loop that follows the **REPL** concept:
1. **Read:** Read the command string from standard input.
2. **Evaluate (Parse):** Break the command string down into a program name and its arguments.
3. **Print (Execute):** Run the parsed command and print any output or errors.
4. **Loop:** Return to step 1.

## The Engine: Fork, Exec, Wait

To execute external commands (like `ls` or `pwd`), the shell interacts with the OS using three essential system calls:
* **`fork()`:** The shell clones itself, creating a "Child" process.
* **`execvp()`:** The Child process replaces its own memory space with the new program you want to run. 
* **`waitpid()`:** The "Parent" process (the shell) waits for the Child to finish executing before printing the next prompt.

---

## Step-by-Step Implementation Plan

### Phase 1: The Foundation (The Loop)
* **Setup:** Create a `main.c` file and set up a `while(1)` loop.
* **The Prompt:** Print a custom prompt indicator (e.g., `myshell> `).
* **Reading Input:** Use `getline()` to read the user's input dynamically. It handles memory allocation, making it easier to read arbitrary-length commands.

### Phase 2: The Parser (Tokenization)
* **Tokenizing:** Use the `strtok()` function to slice the input string into an array of strings (tokens) using spaces as the delimiter.
* **Memory Management:** Maintain an array of pointers (`char **args`) to keep track of these tokens. Ensure the last element in your array is `NULL`, as the `exec` family of functions requires a null-terminated array.

### Phase 3: Execution (Bringing it to Life)
* **Process Spawning:** When a command is entered, call `fork()`.
* **Routing:** * If `fork()` returns `0`, you are in the child process. Call `execvp(args[0], args)`. If `execvp()` returns, the command failed (e.g., command not found), so print an error and `exit()`.
    * If `fork()` returns a positive number, you are in the parent process. Call `waitpid()` to wait for the child's process ID to finish.
    * If `fork()` returns `< 0`, an error occurred during process creation.

### Phase 4: Built-in Commands
Commands like `cd` and `exit` must be built manually because they change the state of the shell itself, not a child process.
* **`exit`:** Return `0` or `break` out of the main loop.
* **`cd`:** Use the `chdir()` system call to change the current working directory.
* **`help`:** Print a simple list of your built-in commands and how the shell works.

### Phase 5: Polish & Core OS Features (Optional)
* **I/O Redirection (`>`, `<`):** Use `open()`, `close()`, and `dup2()` to redirect standard input and output to files.
* **Piping (`|`):** Connect the output of one process to the input of another using the `pipe()` system call.
* **Background Processes (`&`):** Modify `waitpid()` logic so the shell doesn't wait for the child to finish if the command ends with `&`.

### Phase 6: Usability & Quality of Life
* **Signal Handling (Ctrl+C):** Catch `SIGINT` using `signal()` or `sigaction()`. This ensures that pressing `Ctrl+C` kills the running child process (like a runaway loop) rather than exiting the shell itself.
* **Command History (Up/Down Arrows):** Integrate the GNU `readline` library (replacing basic `getline`) to allow scrolling through previously entered commands and better line editing.
* **Tab Auto-completion:** Leverage `readline`'s completion features or write basic `opendir()`/`readdir()` logic to autocomplete file and directory names.

### Phase 7: Advanced Variable & Logic Handling
* **Environment Variables (`export`, `$VAR`):** Create an `export` built-in command. Update the tokenizer to scan for the `$` symbol and expand variables by pulling their values via `getenv()`.
* **Logical Operators (`&&`, `||`):** Parse for conditional separators. Ensure that commands chained with `&&` only run if the previous command succeeded (returned `0`), and `||` runs only if it failed.
* **Globbing (Wildcards):** Add support for `*` and `?`. Use the C `<glob.h>` library to expand patterns (e.g., `*.c`) into lists of matching filenames before sending them to `execvp()`.