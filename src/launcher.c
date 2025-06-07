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
static Token previous(const Self self);
static Token peek(const Self self);
static bool at_end(const Self self);
static bool expect(Self self, const TokenKind kind);
static int last_command_status(const Self self);
static bool is_error(const LauncherState state);
static bool is_symbol(const TokenKind kind);

static char **advance_command(Self self);

static void skip_whitespace(Self self);

static LauncherState launch_statement(Self self);
static LauncherState launch_command(Self self);
static LauncherState launch_if(Self self);

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
    while (!at_end(self)) {
        LauncherState result = launch_statement(self);
        if (is_error(result)) {
            return result;
        }
    }

    return LAUNCHER_SUCCESS;
}

static char **advance_command(Self self) {
    char **args = arrinit(char *);
    Token tok = previous(self);
    while (!at_end(self)) {
        if (!is_symbol(tok.kind)) {
            arrpush(args, nullptr); // let it leak 🗣
            break;
        }
        char *arg = strdup(tok.lexem);
        if (arg == nullptr) {
            for (size_t i = 0; args[i] != nullptr; i++) {
                free(args[i]);
            }
            arrfree(args);
            return nullptr;
        }
        arrpush(args, arg);
        tok = advance(self);
    }

    arrpush(args, nullptr);
    return args;
}

static LauncherState launch_statement(Self self) {
    Token tok = advance(self);
    switch (tok.kind) {
    case TOKEN_IF:
        return launch_if(self);
    case TOKEN_DO:
        fprintf(stderr, "mosh: Syntactic error near `do`.\n");
        return LAUNCHER_SYNTAX_ERROR;
    case TOKEN_END:
        fprintf(stderr, "mosh: Syntactic error near `end`.\n");
        return LAUNCHER_SYNTAX_ERROR;
    case TOKEN_ERROR:
        return LAUNCHER_FAILURE;
    default:
        return launch_command(self);
    }
}

static LauncherState launch_command(Self self) {
    skip_whitespace(self);
    LauncherState result = 0;
    char **args = arrinit(char *);
    Token tok = previous(self);
    while (!at_end(self)) {
        if (!is_symbol(tok.kind)) {
            arrpush(args, nullptr); // let it leak 🗣
            break;
        }
        char *arg = strdup(tok.lexem);
        if (arg == nullptr)
            return LAUNCHER_FAILURE;
        arrpush(args, arg);
        tok = advance(self);
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
    }
    arrfree(args);
    return result;
}

static LauncherState launch_if(Self self) {
    advance(self);
    LauncherState result = LAUNCHER_SUCCESS;
    char **condition = advance_command(self);
    if (condition == nullptr) {
        return LAUNCHER_FAILURE;
    }
    skip_whitespace(self);
    if (!expect(self, TOKEN_DO)) {
        printf("Expected `do` after the condition. got `%s`.\n",
               peek(self).lexem);
        defer(LAUNCHER_SYNTAX_ERROR);
    }

    LauncherState state = execute_wait(self, condition);
    if (is_error(state)) {
        defer(state);
    }

    int status = last_command_status(self);
    if (status == 0) {
        while (!expect(self, TOKEN_END)) {
            if (at_end(self)) {
                printf("Expected `end`.\n");
                defer(LAUNCHER_SYNTAX_ERROR);
            }
            launch_command(self);
        }
    } else {
        while (!expect(self, TOKEN_END)) {
            if (at_end(self)) {
                printf("Expected `end`.\n");
                defer(LAUNCHER_SYNTAX_ERROR);
            }
            advance(self);
        }
    }

defer:
    for (size_t i = 0; condition[i] != nullptr; i++) {
        free(condition[i]);
    }
    arrfree(condition);
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

static bool is_symbol(const TokenKind kind) {
    switch (kind) {
    case TOKEN_EOL:
    case TOKEN_EOF:
    case TOKEN_ERROR:
    case TOKEN_AMPERSAND:
    case TOKEN_SEMICOLON:
        return false;
    case TOKEN_IF:
    case TOKEN_DO:
    case TOKEN_END:
    case TOKEN_SYMBOL:
        return true;
    default:
        unreachable();
    }
}
static bool is_error(const LauncherState state) {
    switch (state) {
    case LAUNCHER_SUCCESS:
    case LAUNCHER_EXIT:
        return false;
    case LAUNCHER_FAILURE:
    case LAUNCHER_SYNTAX_ERROR:
        return true;
    default:
        unreachable();
    }
}
static Token previous(const Self self) { return self->tokenize.prev_tok; }
static Token peek(const Self self) { return self->tokenize.current_tok; }
static bool at_end(const Self self) { return self->tokenize.done; }
static bool expect(Self self, const TokenKind kind) {
    if (peek(self).kind == kind) {
        advance(self);
        return true;
    }
    return false;
}

static int last_command_status(const Self self) {
    const char *status = env_get(self->env, "?");
    if (status == nullptr) {
        return 0;
    }
    return atoi(status);
}

static void skip_whitespace(Self self) {
    Token tok = peek(self);
    while (!at_end(self)) {
        switch (tok.kind) {
        case TOKEN_EOL:
            break;
        default:
            return;
        }
        tok = advance(self);
    }
}

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
