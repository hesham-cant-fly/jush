#include "builtins.h"
#include "environment.h"
#include "my_hash.h"
#include "my_helpers.h"
#include "my_string.h"
#include "my_termcolor.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *builtin_str[] = {
    "cd", "exit", "help", "alias", nullptr,
};

BuiltinStatus (*builtin_func[])(char **, Environment *) = {
    &mosh_cd, &mosh_exit, &mosh_help, &mosh_alias, nullptr,
};

BuiltinStatus mosh_cd(char **args, Environment *env) {
    unused(env);
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

BuiltinStatus mosh_exit(char **args, Environment *env) {
    unused(args);
    unused(env);
    return BUILTIN_EXIT;
}

BuiltinStatus mosh_help(char **args, Environment *env) {
    unused(args);
    unused(env);
    puts("TODO: help");
    return BUILTIN_SUCCESS;
}

BuiltinStatus mosh_alias(char **args, Environment *env) {
    BuiltinStatus result = BUILTIN_SUCCESS;
    if (args[1] == nullptr) {
        fprintf(stderr, "mosh: Expected argument to \"alias\".\n");
        return result;
    }

    if (strcmp(args[1], "-p") == 0) {
        HashMapIter alias_iter = hm_iter(env->aliases);
        Alias *alias = nullptr;
        while ((alias = hm_next(&alias_iter, env->aliases))) {
            printf(ANSI_CODE_BOLD "%s" ANSI_CODE_RESET "=" ANSI_CODE_YELLOW
                                  "\"%s\"" ANSI_CODE_RESET "\n",
                   alias->key, alias->value);
        }
        return result;
    }

    if (strcmp(args[1], "-pro") == 0) {
        hm_print_pro(env->aliases);
        return result;
    }

    if ((strcmp(args[1], "--help") == 0) || (strcmp(args[1], "-h") == 0)) {
        puts("TODO: help");
        return result;
    }

    String name = string_new(20);
    String value = string_new(20);
    size_t i = 0;
    bool creating_alias = false;
    for (; args[1][i] != '\0'; i++) {
        if (args[1][i] == '=') {
            creating_alias = true;
            break;
        }
        string_push(&name, args[1][i]);
    }

    if (!creating_alias) {
        Alias *alias = env_get_alias(env, name.data);
        if (alias == nullptr) {
            fprintf(stderr, "mosh: Undefined alias \"%s\"\n", name.data);
            defer(BUILTIN_FAIL);
        }
        printf(ANSI_CODE_BOLD "%s" ANSI_CODE_RESET "=" ANSI_CODE_YELLOW
                              "\"%s\"" ANSI_CODE_RESET "\n",
               alias->key, alias->value);
        defer(BUILTIN_SUCCESS);
    }

    i += 1;
    for (; args[1][i] != '\0'; i++) {
        string_push(&value, args[1][i]);
    }

    env_set_alias(env, name.data, strdup(value.data));

defer:
    string_delete(&name);
    string_delete(&value);
    return result;
}
