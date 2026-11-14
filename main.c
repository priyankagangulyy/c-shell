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

static void lsh_launch(char **args)
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
}

int main(void)
{
    char *line = NULL;
    size_t line_capacity = 0;

    while (1) {
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

        if (args[0] != NULL) {
            lsh_launch(args);
        }

        free(args);
    }

    free(line);
    return EXIT_SUCCESS;
}
