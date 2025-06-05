#include "environment.h"
#include "my_hash.h"
#include "my_helpers.h"
#include <stddef.h>
#include <stdlib.h>

Environment init_env() {
    Environment result;
    result.aliases = hm_init(Alias);
    result.variables = hm_init(Variable);
    return result;
}

void deinit_env(Environment *self) {
    {
        HashMapIter iter = hm_iter(self->aliases);
        Alias *entry = nullptr;
        while ((entry = hm_next(&iter, self->aliases))) {
            free(entry->value);
        }
        hm_deinit(self->aliases);
    }
    {
        HashMapIter iter = hm_iter(self->variables);
        Variable *entry = nullptr;
        while ((entry = hm_next(&iter, self->variables))) {
            free(entry->value);
        }
        hm_deinit(self->variables);
    }
}

const char *env_get(Environment *self, const char *key) {
    Variable *entry = hm_get_entry(self->variables, (char *)key);
    if (entry != nullptr) {
        return entry->value;
    }
    return getenv(key);
}

void env_set(Environment *self, const char *key, const char *value) {
    hm_set(self->variables, (char *)key, (char *)value);
}

Alias *env_get_alias(Environment *self, const char *key) {
    if (!hm_contains(self->aliases, key))
        return nullptr;
    return hm_get_entry(self->aliases, key);
}

void env_set_alias(Environment *self, const char *key, char *value) {
    hm_set(self->aliases, key, value);
}
