#include "environment.h"
#include "my_hash.h"
#include <stddef.h>
#include <stdio.h>

Environment init_env() {
    Environment result;
    result.aliases = hm_init(Alias);
    return result;
}

void deinit_env(Environment *self) {
    HashMapIter iter = hm_iter(self->aliases);
    Alias *entry = nullptr;
    while ((entry = hm_next(&iter, self->aliases))) {
        free(entry->value);
    }
    hm_deinit(self->aliases);
}

Alias *env_get_alias(Environment *self, const char *key) {
    if (!hm_contains(self->aliases, key))
        return nullptr;
    return hm_get_entry(self->aliases, key);
}

void env_set_alias(Environment *self, const char *key, char *value) {
    hm_set(self->aliases, key, value);
    hm_print_pro(self->aliases);
}
