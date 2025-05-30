#ifndef BUILTINS_H_
#define BUILTINS_H_

typedef enum BuiltinStatus : int {
    BUILTIN_SUCCESS = 0,
    BUILTIN_EXIT,
    BUILTIN_FAIL,
} BuiltinStatus;

BuiltinStatus mosh_cd(char **args);
BuiltinStatus mosh_exit(char **args);

extern char *builtin_str[];

extern int (*builtin_func[])(char **);

#endif // BUILTINS_H_
