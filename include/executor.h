#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "redir.h"

char* find_executable(const char *command);
void execute_command(int argc, char **args, const Redirection *redir);

#endif