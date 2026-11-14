
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

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

        for (size_t i = 0; args[i] != NULL; i++) {
            printf("arg[%zu] = %s\n", i, args[i]);
        }

        free(args);
    }

    free(line);
    return EXIT_SUCCESS;
}
