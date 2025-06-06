#include "launcher.h"
#include "builtins.h"
#include "environment.h"
#include "my_array.h"
#include "my_helpers.h"
#include "my_string.h"
#include "tokenizer.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef Launcher *Self;

static Token advance(Self self);
static Token previous(Self self);
static Token peek(Self self);
static bool at_end(Self self);

Launcher init_launcher(char *source, Environment *env) {
    Launcher result = {
        .env = env,
        .tokenize = init_tokenizer(source, env),
    };
    advance(&result);
    return result;
}

void deinit_launcher(Launcher *self) { deinit_tokenizer(&self->tokenize); }

LauncherState launch(char *source, Environment *env) {
    LauncherState result = LAUNCHER_SUCCESS;
    Launcher launcher = init_launcher(source, env);
    result = launcher_launch(&launcher);
    deinit_launcher(&launcher);
    return result;
}

LauncherState launcher_launch(Launcher *self) {
    LauncherState result = LAUNCHER_SUCCESS;
    char **args = arrinit(char *);

    while (!at_end(self)) {
        arrsetlen(args, 0);
        Token tok = {0};
        while (!at_end(self)) {
            tok = advance(self);
            if (tok.kind != TOKEN_SYMBOL) {
                arrpush(args, nullptr); // let it leak 🗣
                break;
            }
            char *arg = strdup(tok.lexem);
            if (arg == nullptr)
                return LAUNCHER_FAILURE;
            arrpush(args, arg);
        }
        arrpush(args, nullptr);
        switch (tok.kind) {
        case TOKEN_AMPERSAND:
            if (args[0] != nullptr) {
                result = execute_background(self, args);
            }
            break;
        default:
            if (args[0] != nullptr) {
                result = execute_wait(self, args);
            }
        }

        for (size_t i = 0; args[i] != nullptr; i++) {
            free(args[i]);
            args[i] = nullptr;
        }
    }
    for (size_t i = 0; args[i] != nullptr; i++) {
        free(args[i]);
    }
    arrfree(args);
    return result;
}

static Token advance(Self self) {
    loop {
        self->tokenize.current_tok = scan_token(&self->tokenize);
        if (self->tokenize.current_tok.kind != TOKEN_ERROR)
            break;

        fprintf(stderr, "Unexpected character '%c'.\n",
                self->tokenize.source[self->tokenize.current]);
    }

    return previous(self);
}

static Token previous(Self self) { return self->tokenize.prev_tok; }
static Token peek(Self self) { return self->tokenize.current_tok; }
static bool at_end(Self self) { return self->tokenize.done; }

LauncherState execute_wait(Self self, char **args) {
    // Check for aliases
    {
        Alias *alias = env_get_alias(self->env, args[0]);
        if (alias != nullptr) {
            String alias_args = string_from_chars_copy(alias->value);
            string_reserve(&alias_args, alias_args.len + 1);
            for (size_t i = 1; args[i] != nullptr; i++) {
                string_push(&alias_args, (char)' ');
                string_push(&alias_args, args[i]);
            }
            LauncherState result = launch(alias_args.data, self->env);
            string_delete(&alias_args);
            return result;
        }
    }

    // Check for built-in
    for (size_t i = 0; builtin_str[i] != nullptr; i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            BuiltinStatus res = builtin_func[i](args, self->env);
            switch (res) {
            case BUILTIN_FAIL:
                return LAUNCHER_FAILURE;
            case BUILTIN_EXIT:
                return LAUNCHER_EXIT;
            default:
                return LAUNCHER_SUCCESS;
            }
        }
    }

    // Execute the actual command
    return execute_command(args, self->env);
}

// TODO: Support for built-in commands and aliases
LauncherState execute_background(Launcher *self, char **args) {
    unused(self);
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("mosh");
            exit(EXIT_FAILURE);
        }
        printf("[1] Done! %d\n", getpid());
    } else if (pid < 0) {
        perror("mosh");
        return LAUNCHER_FAILURE;
    }

    printf("[1] %d\n", pid);
    char pid_str[20] = {0};
    sprintf(pid_str, "%d", pid);
    env_set(self->env, "!", strdup(pid_str));

    return LAUNCHER_SUCCESS;
}

LauncherState execute_command(char **args, Environment *env) {
    int status = 0;
    pid_t pid = fork(), wpid;
    unused(wpid);
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("mosh");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("mosh");
        return LAUNCHER_FAILURE;
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            if (wpid == -1) {
                perror("mosh");
                return LAUNCHER_FAILURE;
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    char status_str[20] = {0};
    if (WIFEXITED(status)) {
        sprintf(status_str, "%i", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        status_str[0] = '-';
        status_str[1] = '1';
        status_str[2] = '\0';
    }
    env_set(env, "?", strdup(status_str));

    return LAUNCHER_SUCCESS;
}
