#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

typedef struct Alias {
    void *next;
    const char *key;
    char *value;
} Alias;

typedef struct Environment {
    Alias *aliases;
} Environment;

Environment init_env();
void deinit_env(Environment *self);

const char *env_get(Environment *self, const char *key);

Alias *env_get_alias(Environment *self, const char *key);
void env_set_alias(Environment *self, const char *key, char *value);

#endif // ENVIRONMENT_H_
