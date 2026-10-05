#ifndef BUILTINS_H
#define BUILTINS_H

typedef int (*builtin_fn)(int argc, char **argv);

typedef struct {
    const char *name;
    builtin_fn handler;
} BuiltinCommand;

const BuiltinCommand* find_builtin(const char *name);

#endif