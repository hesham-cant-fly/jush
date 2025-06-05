#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

typedef struct Alias {
    void *next;
    const char *key;
    char *value;
} Alias;

typedef struct Variable {
    void *next;
    const char *key;
    char *value;
} Variable;

typedef struct Environment {
    Alias *aliases;
    Variable *variables;
} Environment;

Environment init_env();
void deinit_env(Environment *self);

const char *env_get(Environment *self, const char *key);
void env_set(Environment *self, const char *key, const char *value);

Alias *env_get_alias(Environment *self, const char *key);
void env_set_alias(Environment *self, const char *key, char *value);

#endif // ENVIRONMENT_H_
