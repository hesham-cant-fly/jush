#include "builtins.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char *builtin_str[] = {
    "cd",
    "exit",
    nullptr,
};

int (*builtin_func[])(char **) = {
    &mosh_cd,
    &mosh_exit,
    nullptr,
};

BuiltinStatus mosh_cd(char **args) {
    if (args[1] == nullptr) {
        fprintf(stderr, "mosh: Expected argument to \"cd\".\n");
        return BUILTIN_FAIL;
    }
    if (chdir(args[1]) != 0) {
        perror("mosh");
        return BUILTIN_FAIL;
    }
    return BUILTIN_SUCCESS;
}

BuiltinStatus mosh_exit(char **args) {
    /* exit(0); */
    return BUILTIN_EXIT;
}
