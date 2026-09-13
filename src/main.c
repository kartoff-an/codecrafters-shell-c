#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_LENGTH 100

int find_type(char* type, const char* types[], int n_types) {
  for (int i = 0; i < n_types; i++) {
    if (strcmp(types[i], type) == 0) {
      return i;
    }
  }
  return -1;
}

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char command[MAX_COMMAND_LENGTH];
  const char* types[] = {"echo", "exit", "type"};
  int types_size = sizeof(types) / sizeof(types[0]);

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
      if (find_type(arg, types, types_size) >= 0) {
        printf("%s is a shell builtin\n", arg);
      }
      else {
        printf("%s: not found\n", arg);
      }
    }
    else {
      printf("%s: command not found\n", builtin);
    }
  }

  return 0;
}
