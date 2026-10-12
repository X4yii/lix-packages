#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char *initial_env[] = { NULL };
char **environ = initial_env;

char *getenv(const char *name) {
    if (!name || !environ) return NULL;
    size_t name_len = strlen(name);
    for (char **ep = environ; *ep; ep++) {
        if (strncmp(*ep, name, name_len) == 0 && (*ep)[name_len] == '=') {
            return *ep + name_len + 1;
        }
    }
    return NULL;
}

int setenv(const char *name, const char *value, int overwrite) {
    if (!name || !*name || strchr(name, '=') || !value) {
        errno = EINVAL;
        return -1;
    }
    if (!overwrite && getenv(name)) {
        return 0;
    }
    size_t name_len = strlen(name);
    size_t val_len = strlen(value);
    char *new_str = (char *)malloc(name_len + 1 + val_len + 1);
    if (!new_str) {
        errno = ENOMEM;
        return -1;
    }
    memcpy(new_str, name, name_len);
    new_str[name_len] = '=';
    memcpy(new_str + name_len + 1, value, val_len + 1);

    size_t count = 0;
    if (environ) {
        while (environ[count]) count++;
    }

    for (size_t i = 0; i < count; i++) {
        if (strncmp(environ[i], name, name_len) == 0 && environ[i][name_len] == '=') {
            environ[i] = new_str;
            return 0;
        }
    }

    char **new_env = (char **)malloc((count + 2) * sizeof(char *));
    if (!new_env) {
        free(new_str);
        errno = ENOMEM;
        return -1;
    }
    for (size_t i = 0; i < count; i++) {
        new_env[i] = environ[i];
    }
    new_env[count] = new_str;
    new_env[count + 1] = NULL;
    environ = new_env;
    return 0;
}

int unsetenv(const char *name) {
    if (!name || !*name || strchr(name, '=') || !environ) {
        errno = EINVAL;
        return -1;
    }
    size_t name_len = strlen(name);
    char **ep = environ;
    char **dp = environ;
    while (*ep) {
        if (strncmp(*ep, name, name_len) == 0 && (*ep)[name_len] == '=') {
            ep++;
        } else {
            *dp++ = *ep++;
        }
    }
    *dp = NULL;
    return 0;
}
