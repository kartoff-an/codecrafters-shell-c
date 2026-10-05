#define _POSIX_C_SOURCE 200809L
#include "executor.h"
#include "builtins.h"
#include "redir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

#ifndef PATH_LIST_DELIM
#define PATH_LIST_DELIM ":"
#endif

#ifndef DIR_SEPARATOR_STR
#define DIR_SEPARATOR_STR "/"
#endif

char* find_executable(const char *command) {
    const char *path = getenv("PATH");
    if (path == NULL || *path == '\0') {
        return NULL;
    }

    char *path_copy = strdup(path);
    if (path_copy == NULL) {
        return NULL;
    }

    char full_path[PATH_MAX];
    char *saveptr = NULL;
    char *dir = strtok_r(path_copy, PATH_LIST_DELIM, &saveptr);

    while (dir != NULL) {
        snprintf(full_path, sizeof(full_path), "%s" DIR_SEPARATOR_STR "%s", dir, command);
        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }
        dir = strtok_r(NULL, PATH_LIST_DELIM, &saveptr);
    }

    free(path_copy);
    return NULL;
}

static void execute_external(char **args, const Redirection *redir) {
    char *full_path = find_executable(args[0]);
    if (full_path == NULL) {
        printf("%s: command not found\n", args[0]);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        apply_redirection(redir, NULL);
        execv(full_path, args);
        perror("execv");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
    else {
        perror("fork");
    }

    free(full_path);
}

void execute_command(int argc, char **args, const Redirection *redir) {
    if (argc == 0) return;

    const BuiltinCommand *builtin = find_builtin(args[0]);
    if (builtin != NULL) {
        RedirBackup backup;
        apply_redirection(redir, &backup);
        builtin->handler(argc, args);
        restore_redirection(&backup);
    }
    else {
        execute_external(args, redir);
    }
}