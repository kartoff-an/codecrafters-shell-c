#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include "redir.h"
#include "executor.h"
#include "completion.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 1024

int main(void) {
  completion_init();

  setbuf(stdout, NULL);

  char *input;
  char *args[MAX_ARGS];

  while (1) {
    input = readline("$ ");
    if (input == NULL) {
      break;
    }

    if (*input != '\0') {
      add_history(input);
    }

    int argc = parse_input(input, args, MAX_ARGS);
    if (argc == 0) {
      continue;
    }

    Redirection redir;
    extract_redirections(args, &argc, &redir);

    execute_command(argc, args, &redir);

    free_redirection(&redir);
    free_args(args, argc);
  }
}