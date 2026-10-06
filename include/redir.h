#ifndef REDIR_H
#define REDIR_H

typedef enum {
    REDIR_TRUNC,
    REDIR_APPEND
} RedirMode;

typedef struct {
    char *stdout_path;
    char *stderr_path;
    RedirMode stdout_mode;
    RedirMode stderr_mode;
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