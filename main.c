#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static char **lsh_split_line(char *line)
{
    size_t capacity = 8;
    size_t count = 0;
    char **tokens = malloc(capacity * sizeof *tokens);

    if (tokens == NULL) {
        return NULL;
    }

    char *token = strtok(line, " \t\r\n");

    while (token != NULL) {
        if (count + 1 >= capacity) {
            capacity *= 2;
            char **grown = realloc(tokens, capacity * sizeof *tokens);

            if (grown == NULL) {
                free(tokens);
                return NULL;
            }

            tokens = grown;
        }

        tokens[count++] = token;
        token = strtok(NULL, " \t\r\n");
    }

    tokens[count] = NULL;
    return tokens;
}

static int lsh_launch(char **args)
{
    pid_t pid = fork();

    if (pid == 0) {
        execvp(args[0], args);
        perror(args[0]);
        _exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
    } else {
        int status;

        while (waitpid(pid, &status, 0) == -1) {
            if (errno == EINTR) {
                continue;
            }

            perror("waitpid");
            break;
        }
    }

    return 1;
}

static int lsh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected a directory after cd\n");
    } else if (chdir(args[1]) != 0) {
        perror("cd");
    }

    return 1;
}

static int lsh_help(char **args)
{
    (void)args;
    puts("Built-in commands: cd, help, exit");
    puts("Other commands are searched for using PATH.");
    return 1;
}

static int lsh_exit(char **args)
{
    (void)args;
    return 0;
}

static int lsh_execute(char **args)
{
    if (args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "cd") == 0) {
        return lsh_cd(args);
    }
    if (strcmp(args[0], "help") == 0) {
        return lsh_help(args);
    }
    if (strcmp(args[0], "exit") == 0) {
        return lsh_exit(args);
    }

    return lsh_launch(args);
}

int main(void)
{
    char *line = NULL;
    size_t line_capacity = 0;
    int status = 1;

    while (status) {
        printf("lsh> ");
        fflush(stdout);

        ssize_t length = getline(&line, &line_capacity, stdin);
        if (length == -1) {
            putchar('\n');
            break;
        }

        char **args = lsh_split_line(line);
        if (args == NULL) {
            perror("lsh_split_line");
            break;
        }

        status = lsh_execute(args);
        free(args);
    }

    free(line);
    return EXIT_SUCCESS;
}
