# lsh: A Simple C Shell

A small Unix-style shell written in C as a systems programming project.

## Features

- Reads and splits command input on whitespace
- Runs external programs found through `PATH`
- Includes `cd`, `help`, and `exit` built-in commands
- Waits for each external command to finish

## Build and run

```bash
gcc -Wall -Wextra -std=c11 -o lsh main.c
./lsh
