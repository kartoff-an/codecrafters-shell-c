#define _POSIX_C_SOURCE 200809L
#include "parser.h"

#include <stdlib.h>
#include <string.h>

int parse_input(const char *input, char **args, int max_args) {
    int argc = 0;
    int in_single_quote = 0;
    int in_double_quote = 0;
    char token[MAX_TOKEN_LEN];
    int token_len = 0;
    int has_token = 0;

    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i];

        if (in_single_quote) {
            if (c == '\'') {
                in_single_quote = 0;
            }
            else if (token_len < MAX_TOKEN_LEN - 1) {
                token[token_len++] = c;
            }
        }
        else if (in_double_quote) {
            if (c == '"') {
                in_double_quote = 0;
            }
            else {
                if (c == '\\' && (input[i + 1] == '"' || input[i + 1] == '\\')) {
                    c = input[++i];
                }
                if (token_len < MAX_TOKEN_LEN - 1) {
                    token[token_len++] = c;
                }
            }
        }
        else {
            if (c == '\\') {
                if (input[i + 1] != '\0') {
                    if (token_len < MAX_TOKEN_LEN - 1) {
                        token[token_len++] = input[++i];
                    }
                    has_token = 1;
                }
            }
            else if (c == '\'') {
                in_single_quote = 1;
                has_token = 1;
            }
            else if (c == '"') {
                in_double_quote = 1;
                has_token = 1;
            }
            else if (c == ' ' || c == '\t' || c == '\n') {
                if (has_token) {
                    token[token_len] = '\0';
                    if (argc < max_args - 1) {
                        args[argc++] = strdup(token);
                    }
                    token_len = 0;
                    has_token = 0;
                }
            }
            else {
                if (token_len < MAX_TOKEN_LEN - 1) {
                    token[token_len++] = c;
                }
                has_token = 1;
            }
        }
    }

    if (has_token) {
        token[token_len] = '\0';
        if (argc < max_args - 1) {
            args[argc++] = strdup(token);
        }
    }

    args[argc] = NULL;
    return argc;
}

void free_args(char **args, int argc) {
    for (int i = 0; i < argc; i++) {
        free(args[i]);
        args[i] = NULL;
    }
}