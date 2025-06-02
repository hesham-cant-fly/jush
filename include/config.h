#ifndef CONFIG_H_
#define CONFIG_H_

#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MOSH_DEFAULT_PROMPET "$"
#define MOSH_DEFAULT_VERSION "0.0.1"
#define MOSH_DEFAULT_LEN 50
#define MOSH_DEFAULT_CONFIG_PATH "./.moshrc"

char *get_default_prompet() {
    char *buf = getcwd(nullptr, 0);
    char *the_basename = basename(buf);
    if (buf == nullptr) {
        return nullptr;
    }
    size_t len = snprintf(NULL, 0, "[%s %s/]%s ", getenv("USER"), the_basename,
                          MOSH_DEFAULT_PROMPET);
    char *out = malloc(len + 1);
    if (out == nullptr) {
        free(buf);
        return nullptr;
    }
    snprintf(out, len + 1, "[%s %s/]%s ", getenv("USER"), the_basename,
             MOSH_DEFAULT_PROMPET);
    free(buf);
    return out;
}

#endif // CONFIG_H_
