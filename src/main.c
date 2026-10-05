#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include "redir.h"
#include "executor.h"

#include <stdio.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 1024

int main(void) {
  setbuf(stdout, NULL);

  char input[MAX_COMMAND_LENGTH];
  char *args[MAX_ARGS];

  while (1) {
    printf("$ ");

    if (fgets(input, sizeof(input), stdin) == NULL) {
      break;
    }
    input[strcspn(input, "\n")] = '\0';

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