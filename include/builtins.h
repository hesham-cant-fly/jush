#ifndef BUILTINS_H_
#define BUILTINS_H_

#include "environment.h"
typedef enum BuiltinStatus : int {
    BUILTIN_SUCCESS = 0,
    BUILTIN_EXIT,
    BUILTIN_FAIL,
} BuiltinStatus;

extern char *builtin_str[];
extern BuiltinStatus (*builtin_func[])(char **, Environment *);

BuiltinStatus mosh_cd(char **args, Environment *env);
BuiltinStatus mosh_exit(char **args, Environment *env);
BuiltinStatus mosh_help(char **args, Environment *env);
BuiltinStatus mosh_alias(char **args, Environment *env);
BuiltinStatus mosh_command(char **args, Environment *env);

#endif // BUILTINS_H_
