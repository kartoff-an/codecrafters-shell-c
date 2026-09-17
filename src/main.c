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

int main(int argc, char *argv[]) {
  // Flush after every printf
  setbuf(stdout, NULL);

  char input[MAX_COMMAND_LENGTH];

  int should_exit = 0;
  while (!should_exit) {
    printf("$ ");

    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';
    char *cmd = strtok(input, " ");
    char *arg = strtok(NULL, "");

    if (cmd == NULL) {
      continue;
    }

    if (strcmp(cmd, "exit") == 0) {
      break;
    }
    else if (strcmp(cmd, "echo") == 0) {
      printf("%s\n", arg);
    }
    else if (strcmp(cmd, "pwd") == 0) {
      char cwd[FILENAME_MAX];
      getcwd(cwd, sizeof(cwd));
      printf("%s\n", cwd);
    }
    else if (strcmp(cmd, "cd") == 0) {
      if (chdir(arg) != 0 || errno == ENOENT) {
        printf("cd: %s: No such file or directory\n", arg);
      }
    }
    else if (strcmp(cmd, "type") == 0) {
      handle_type(arg);
    }
    else {
      char *full_path = find_executable(cmd);
      if (full_path != NULL) {
        char *exec_args[MAX_ARGS];
        int arg_idx = 0;
        exec_args[arg_idx++] = cmd;

        if (arg != NULL) {
          char *token = strtok(arg, " ");
          while (token != NULL && arg_idx < MAX_ARGS - 1) {
            exec_args[arg_idx++] = token;
            token = strtok(NULL, " ");
          }
        }
        exec_args[arg_idx] = NULL;

        pid_t pid = fork();
        if (pid == 0) {
          execv(full_path, exec_args);
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
  }

  return 0;
}
