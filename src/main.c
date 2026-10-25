#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include "redir.h"
#include "executor.h"
#include "builtins.h"

#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 1024

static char *complete_builtin(const char *text, int state) {
  static int index;
  size_t length;
  const char **builtins;

  if (state == 0) {
    index = 0;
  }

  length = strlen(text);
  builtins = get_builtin_names();

  while(builtins[index] != NULL) {
    const char *name = builtins[index++];

    if (strncmp(name, text, length) == 0) {
      return strdup(name);
    }
  }

  return NULL;
}

static char **shell_completion(const char *text, int start, int end) {
  (void)end;

  if (start != 0) {
    return NULL;
  }

  return rl_completion_matches(text, complete_builtin);
}

int main(void) {
  setbuf(stdout, NULL);

  char *input;
  char *args[MAX_ARGS];

  rl_attempted_completion_function = shell_completion;
  rl_completion_append_character = ' ';

  while (1) {
    input = readline("kart> ");
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