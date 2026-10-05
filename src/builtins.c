#define _POSIX_C_SOURCE 200809L
#include "builtins.h"
#include "executor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

static int builtin_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (i > 1) printf(" ");
        printf("%s", argv[i]);
    }
    printf("\n");
    return 0;
}

static int builtin_pwd(int argc, char **argv) {
    (void)argc;
    (void)argv;
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    }
    return 0;
}

static int builtin_cd(int argc, char **argv) {
    char *target = (argc > 1) ? argv[1] : getenv("HOME");
    if (target != NULL && strcmp(target, "~") == 0) {
        target = getenv("HOME");
    }
    if (target == NULL || chdir(target) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", (argc > 1) ? argv[1] : "");
        return 1;
    }
    return 0;
}

static int builtin_exit(int argc, char **argv) {
    int code = (argc > 1) ? atoi(argv[1]) : 0;
    exit(code);
}

static const BuiltinCommand builtins[];

static int builtin_type(int argc, char **argv) {
    if (argc < 2) return 0;
    const char *name = argv[1];

    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(builtins[i].name, name) == 0) {
            printf("%s is a shell builtin\n", name);
            return 0;
        }
    }

    char *path = find_executable(name);
    if (path != NULL) {
        printf("%s is %s\n", name, path);
        free(path);
        return 0;
    }

    printf("%s: not found\n", name);
    return 1;
}

static const BuiltinCommand builtins[] = {
    {"echo",    builtin_echo},
    {"pwd",     builtin_pwd},
    {"cd",      builtin_cd},
    {"type",    builtin_type},
    {"exit",    builtin_exit},
    {NULL,      NULL}
};

const BuiltinCommand* find_builtin(const char *name) {
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(builtins[i].name, name) == 0) {
            return &builtins[i];
        }
    }
    return NULL;
}