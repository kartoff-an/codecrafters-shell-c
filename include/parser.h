#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64
#define MAX_TOKEN_LEN 1024

int parse_input(const char *input, char **args, int max_args);
void free_args(char **args, int argc);

#endif