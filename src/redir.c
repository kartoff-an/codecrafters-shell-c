#define _POSIX_C_SOURCE 200809L
#include "redir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

void extract_redirections(char **args, int *argc, Redirection *redir) {
    redir->stdout_path = NULL;
    redir->stderr_path = NULL;

    int write_idx = 0;
    for (int i = 0; i < *argc; i++) {
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], "1>") == 0) {
            if (i + 1 < *argc) {
                free(redir->stdout_path);
                redir->stdout_path = strdup(args[i + 1]);
                free(args[i]);
                free(args[i + 1]);
                i++;
            }
            else {
                free(args[i]);
            }
        }
        else if (strcmp(args[i], "2>") == 0) {
            if (i + 1 < *argc) {
                free(redir->stderr_path);
                redir->stderr_path = strdup(args[i + 1]);
                free(args[i]);
                free(args[i + 1]);
                i++;
            }
            else {
                free(args[i]);
            }
        }
        else {
            args[write_idx++] = args[i];
        }
    }

    args[write_idx] = NULL;
    *argc = write_idx;
}

void apply_redirection(const Redirection *redir, RedirBackup *backup) {
    if (backup) {
        backup->saved_stdout = -1;
        backup->saved_stderr = -1;
    }

    if (redir->stdout_path) {
        int fd = open(redir->stdout_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd >= 0) {
            if (backup) {
                backup->saved_stdout = dup(STDERR_FILENO);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        else {
            perror("open");
        }
    }

    if (redir->stderr_path) {
        int fd = open(redir->stderr_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd >= 0) {
            if (backup) {
                backup->saved_stderr = dup(STDERR_FILENO);
            }
            dup2(fd, STDERR_FILENO);
            close(fd);
        } else {
            perror("open");
        }
    }
}

void restore_redirection(const RedirBackup *backup) {
    if (!backup) return;

    if (backup->saved_stdout != -1) {
        fflush(stdout);
        dup2(backup->saved_stdout, STDOUT_FILENO);
        close(backup->saved_stdout);
    }
    if (backup->saved_stderr != -1) {
        fflush(stderr);
        dup2(backup->saved_stderr, STDERR_FILENO);
        close(backup->saved_stderr);
    }
}

void free_redirection(Redirection *redir) {
    if (!redir) return;
    free(redir->stdout_path);
    free(redir->stderr_path);
    redir->stdout_path = NULL;
    redir->stderr_path = NULL;
}