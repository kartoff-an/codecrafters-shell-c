#define _POSIX_C_SOURCE 200809L

#include "completion.h"
#include "builtins.h"

#include <readline/readline.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef PATH_LIST_DELIM
#define PATH_LIST_DELIM ":"
#endif

#ifndef DIR_SEPARATOR_STR
#define DIR_SEPARATOR_STR "/"
#endif

static char **matches;
static size_t match_count;
static size_t match_index;

static void free_matches(void) {
    for (size_t i = 0; i < match_count; i++) {
        free(matches[i]);
    }

    free(matches);
    matches = NULL;
    match_count = 0;
    match_index = 0;
}

static int has_match(const char *name) {
    for (size_t i = 0; i < match_count; i++) {
        if (strcmp(matches[i], name) == 0) {
            return 1;
        }
    }

    return 0;
}

/* Add a candidate if it matches the prefix and is not a duplicate. */
static void add_match(const char *name, const char *prefix) {
    size_t prefix_length = strlen(prefix);

    if (strncmp(name, prefix, prefix_length) != 0 || has_match(name)) {
        return;
    }

    char *copy = strdup(name);
    if (copy == NULL) {
        return;
    }

    char **new_matches = realloc(matches, (match_count + 1) * sizeof(*matches));

    if (new_matches == NULL) {
        free(copy);
        return;
    }

    matches = new_matches;
    matches[match_count++] = copy;
}

/* Find executable files in every directory listed in PATH. */
static void collect_executables(const char *prefix) {
    const char *path = getenv("PATH");
    if (path == NULL) {
        return;
    }

    size_t prefix_length = strlen(prefix);
    const char *current = path;

    while (1) {
        const char *separator = strchr(current, ':');
        size_t directory_length = separator ? (size_t)(separator - current) : strlen(current);

        char *directory;

        if (directory_length == 0) {
            directory = strdup(".");
        } else {
            directory = strndup(current, directory_length);
        }

        if (directory != NULL) {
            DIR *dp = opendir(directory);

            /* Missing or inaccessible directories are safely skipped. */
            if (dp != NULL) {
                struct dirent *entry;
                while ((entry = readdir(dp)) != NULL) {
                    const char *name = entry->d_name;

                    if (strncmp(name, prefix, prefix_length) != 0) {
                        continue;
                    }

                    size_t full_length = strlen(directory) + strlen(name) + 2;

                    char *full_path = malloc(full_length);
                    if (full_path == NULL) {
                        continue;
                    }

                    snprintf(full_path, full_length, "%s" DIR_SEPARATOR_STR "%s", directory, name);

                    struct stat info;

                    if (stat(full_path, &info) == 0 && S_ISREG(info.st_mode) && access(full_path, X_OK) == 0) {
                        add_match(name, prefix);
                    }

                    free(full_path);
                }

                closedir(dp);
            }

            free(directory);
        }

        if (separator == NULL) {
            break;
        }

        current = separator + 1;
    }
}

/* Readline calls this repeatedly to retrieve completion candidates. */
static char *command_generator(const char *text, int state) {
    if (state == 0) {
        free_matches();

        const char **names = get_builtin_names();

        if (names != NULL) {
            for (size_t i = 0; names[i] != NULL; i++) {
                add_match(names[i], text);
            }
        }

        collect_executables(text);
    }

    if (match_index < match_count) {
        return strdup(matches[match_index++]);
    }

    free_matches();
    return NULL;
}

/* Complete commands only at the beginning of the input line. */
static char **shell_completion(const char *text, int start, int end) {
    (void)end;

    if (start != 0) {
        return NULL;
    }

    return rl_completion_matches(text, command_generator);
}

/* Register the completion callback with GNU Readline. */
void completion_init(void) {
    rl_attempted_completion_function = shell_completion;
    rl_completion_append_character = ' ';
}
