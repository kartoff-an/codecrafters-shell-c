#ifndef REDIR_H
#define REDIR_H

typedef struct {
    char *stdout_path;
    char *stderr_path;
} Redirection;

typedef struct {
    int saved_stdout;
    int saved_stderr;
} RedirBackup;

void extract_redirections(char **args, int *argc, Redirection *redir);
void apply_redirection(const Redirection *redir, RedirBackup *backup);
void restore_redirection(const RedirBackup *backup);
void free_redirection(Redirection *redir);

#endif