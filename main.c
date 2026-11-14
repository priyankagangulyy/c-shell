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

static void lsh_wait_for(pid_t pid)
{
    int status;

    while (waitpid(pid, &status, 0) == -1) {
        if (errno == EINTR) {
            continue;
        }

        perror("waitpid");
        break;
    }
}

static int lsh_launch(char **args)
{
    pid_t pid = fork();

    if (pid == 0) {
        execvp(args[0], args);
        perror(args[0]);
        _exit(EXIT_FAILURE);
    }

    if (pid < 0) {
        perror("fork");
    } else {
        lsh_wait_for(pid);
    }

    return 1;
}

static int lsh_pipeline(char **left, char **right)
{
    int fds[2];

    if (pipe(fds) == -1) {
        perror("pipe");
        return 1;
    }

    pid_t left_pid = fork();

    if (left_pid == 0) {
        if (dup2(fds[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(EXIT_FAILURE);
        }

        close(fds[0]);
        close(fds[1]);
        execvp(left[0], left);
        perror(left[0]);
        _exit(EXIT_FAILURE);
    }

    if (left_pid < 0) {
        perror("fork");
        close(fds[0]);
        close(fds[1]);
        return 1;
    }

    pid_t right_pid = fork();

    if (right_pid == 0) {
        if (dup2(fds[0], STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(EXIT_FAILURE);
        }

        close(fds[0]);
        close(fds[1]);
        execvp(right[0], right);
        perror(right[0]);
        _exit(EXIT_FAILURE);
    }

    close(fds[0]);
    close(fds[1]);

    if (right_pid < 0) {
        perror("fork");
        lsh_wait_for(left_pid);
        return 1;
    }

    lsh_wait_for(left_pid);
    lsh_wait_for(right_pid);
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

    size_t pipe_index = 0;
    int pipe_found = 0;

    for (size_t i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            if (pipe_found) {
                fprintf(stderr, "lsh: only one pipe is supported for now\n");
                return 1;
            }

            pipe_index = i;
            pipe_found = 1;
        }
    }

    if (pipe_found) {
        if (pipe_index == 0 || args[pipe_index + 1] == NULL) {
            fprintf(stderr, "lsh: put a command on both sides of |\n");
            return 1;
        }

        args[pipe_index] = NULL;
        return lsh_pipeline(args, &args[pipe_index + 1]);
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
