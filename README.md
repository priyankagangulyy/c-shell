cat > README.md <<'EOF'
# lsh: A Simple C Shell

A small Unix-style shell written in C as a systems programming project.

## Features

- Runs external commands found through `PATH`
- Includes the built-ins `cd`, `help`, and `exit`
- Supports one pipe between two external commands
- Supports input redirection with `<`
- Supports output redirection with `>`

## Build and run

```bash
gcc -Wall -Wextra -std=c11 -o lsh main.c
./lsh
```

## Examples

```text
lsh> pwd
lsh> ls | wc -l
lsh> cat < input.txt
lsh> echo hello > output.txt
lsh> exit
```

Put spaces around `|`, `<`, and `>`.

## Current limitations

Quoted arguments, multiple pipes, combining pipes with redirection, and background jobs are not supported yet.
EOF






