#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(void)
{
    char *line = NULL;
    size_t capacity = 0;

    while (1) {
        printf("lsh> ");
        fflush(stdout);

        ssize_t length = getline(&line, &capacity, stdin);
        if (length == -1) {
            putchar('\n');
            break;
        }

        printf("You typed: %s", line);
    }

    free(line);
    return EXIT_SUCCESS;
}
