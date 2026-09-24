#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/wait.h>

#ifndef PATH_LIST_DELIM
#define PATH_LIST_DELIM ":"
#endif

#ifndef DIR_SEPARATOR_STR
#define DIR_SEPARATOR_STR "/"
#endif

#define MAX_COMMAND_LENGTH 100
#define MAX_ARGS 64
#define MAX_TOKEN_LEN 1024

static const char *const commands[] = {"echo", "exit", "type", "pwd", "cd", NULL};

char* find_executable(char* command) {
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

void handle_type(char* command) {
  for (int i = 0; commands[i] != NULL; i++) {
    if (strcmp(commands[i], command) == 0) {
      printf("%s is a shell builtin\n", command);
      return;
    }
  }

  char* full_path = find_executable(command);
  if (full_path != NULL) {
    printf("%s is %s\n", command, full_path);
    return;
  }

  printf("%s: not found\n", command);
}

int parse_input(const char *input, char **args, int max_args) {
  int argc = 0;
  int in_single_quote = 0;
  int in_double_quote = 0;
  char token[MAX_TOKEN_LEN];
  int token_len = 0;
  int has_token = 0;

  // const char *const special_chars[] = {'"', '\\', '$', '`'};

  for (int i = 0; input[i] != '\0'; i++) {
    char c = input[i];

    if (in_single_quote) {
      if (c == '\'') {
        in_single_quote = 0;
      }
      else {
        if (token_len < MAX_TOKEN_LEN - 1) {
          token[token_len++] = c;
        }
      }
    }
    else if (in_double_quote) {
      if (c == '"') {
        in_double_quote = 0;
      }
      else {
        if (c == '\\') {
          if (input[i + 1] == '"' || c == '\\') {
            c = input[++i];
          }
        }
        if (token_len < MAX_TOKEN_LEN - 1) {
          token[token_len++] = c;
        }
      }
    }
    else {
      if (c == '\\') {
        if (input[i + 1] != '\0') {
          i++;
          if (token_len < MAX_TOKEN_LEN - 1) {
            token[token_len++] = input[i];
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

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char input[MAX_COMMAND_LENGTH];
  char *args[MAX_ARGS];

  int should_exit = 0;
  while (!should_exit) {
    printf("$ ");

    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';
    
    int parsed_argc = parse_input(input, args, MAX_ARGS);
    if (parsed_argc == 0) {
      continue;
    }

    char *cmd = args[0];

    if (strcmp(cmd, "exit") == 0) {
      int exit_code = (parsed_argc > 1) ? atoi(args[1]) : 0;
      free_args(args, parsed_argc);
      exit(exit_code);
    }
    else if (strcmp(cmd, "echo") == 0) {
      for (int i = 1; i < parsed_argc; i++) {
        if (i > 1) {
          printf(" ");
        }
        printf("%s", args[i]);
      }
      printf("\n");
    }
    else if (strcmp(cmd, "pwd") == 0) {
      char cwd[FILENAME_MAX];
      if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
      }
    }
    else if (strcmp(cmd, "cd") == 0) {
      if (parsed_argc > 1) {
        char *target = args[1];
        if (strcmp(target, "~") == 0) {
          target = getenv("HOME");
        }
        if (target == NULL || chdir(target) != 0) {
          printf("cd: %s: No such file or directory\n", args[1]);
        }
      }
    }
    else if (strcmp(cmd, "type") == 0) {
      if (parsed_argc > 1) {
        handle_type(args[1]);
      }
    }
    else {
      char *full_path = find_executable(cmd);
      if (full_path != NULL) {
        pid_t pid = fork();
        if (pid == 0) {
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
      } else {
        printf("%s: command not found\n", cmd);
      }
    }

    free_args(args, parsed_argc);
  }

  return 0;
}
