# HY345 Shell – Mini UNIX Shell in C

A lightweight shell implementation written in C for educational purposes, developed for the **HY345 Operating Systems** course at the University of Crete.

This shell supports:

- Multiple commands with `;`
- Piping (`|`) between commands
- Input/output redirection (`<`, `>`, `>>`)
- Dynamic input reading
- Basic error handling
- Custom command prompt
- Exit command via "bye"

---

## 🧠 Features

- **Prompt Display**: Shows current user and working directory
- **Command Execution**: Executes both built-in and external commands
- **Pipes**: Supports piping output between multiple processes
- **Redirection**: Handles `<`, `>`, and `>>` for file input/output
- **Multiple Commands**: Accepts and executes commands separated by `;`
- **Memory-Safe Input**: Uses dynamic buffers for arbitrary-length input
- **Exit Support**: Type `"bye"` to quit the shell

---

## 📦 Compilation

```bash
make all
