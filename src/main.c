#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define MAX_COMMAND_LENGTH 100

static const char *const commands[] = {"echo", "exit", "type", NULL};

void handle_type(char* command) {
  for (int i = 0; commands[i] != NULL; i++) {
    if (strcmp(commands[i], command) == 0) {
      printf("%s is a shell builtin\n", command);
      return;
    }
  }

  const char *path = getenv("PATH");
  if (path == NULL) {
    printf("%s: not found\n", command);
    return;
  }

  char *path_copy = strdup(path);
  if (path_copy == NULL) {
    perror("strdup");
    return;
  }

  char full_path[PATH_MAX];
  char *dir = strtok(path_copy, ":");

  while (dir != NULL) {
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
    if (access(full_path, X_OK) == 0) {
      printf("%s is %s\n", command, full_path);
      free(path_copy);
      return;
    }
    dir = strtok(NULL, ":");
  }

  free(path_copy);
  printf("%s: not found\n", command);
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char command[MAX_COMMAND_LENGTH];

  int should_exit = 0;
  while (!should_exit) {
    printf("$ ");

    fgets(command, sizeof(command), stdin);
    command[strcspn(command, "\n")] = '\0';
    char *builtin = strtok(command, " ");
    char *arg = strtok(NULL, "");

    if (builtin == NULL) {
      continue;
    }

    if (strcmp(builtin, "exit") == 0) {
      break;
    }
    else if (strcmp(builtin, "echo") == 0) {
      printf("%s\n", arg);
    }
    else if (strcmp(builtin, "type") == 0) {
      handle_type(arg);
    }
    else {
      printf("%s: command not found\n", builtin);
    }
  }

  return 0;
}
