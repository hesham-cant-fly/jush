#ifndef LAUNCHER_H_
#define LAUNCHER_H_

#include "environment.h"
#include "tokenizer.h"

typedef enum LauncherState : int {
    LAUNCHER_SUCCESS,
    LAUNCHER_EXIT,
    LAUNCHER_FAILURE,
    LAUNCHER_SYNTAX_ERROR,
} LauncherState;

typedef struct Launcher {
    Tokenizer tokenize;
    Environment *env;
} Launcher;

Launcher init_launcher(char *source, Environment *env);
void deinit_launcher(Launcher *self);
LauncherState launcher_launch(Launcher *self);
LauncherState launch(char *source, Environment *env);
LauncherState execute_wait(Launcher *self, char **args);
LauncherState execute_background(Launcher *self, char **args);
LauncherState execute_command(char **args, Environment *env);

#endif // LAUNCHER_H_
