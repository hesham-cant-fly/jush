#include "parser.h"
#include "builtins.h"
#include "my_array.h"
#include "my_helpers.h"
#include "my_string.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static Token scan_token(Parser *self);
static inline Token parser_peek(Parser *self) { return self->current; }
static inline Token parser_prev(Parser *self) { return self->prev; }
static Token parser_advance(Parser *self);
static Token make_token(Parser *self, TokenKind kind);

void init_parser(Parser *self) {
    memset(self, 0, sizeof(Parser));
    self->source = nullptr;
    self->start = nullptr;
    self->current_ch = nullptr;
    self->current_lexem = string_new(30);
    self->prev_lexem = string_new(30);
    self->at_end = false;
}

void reinit_parser(Parser *self, const char *source) {
    self->source = source;
    self->start = (char *)source;
    self->current_ch = (char *)source;
    self->current_lexem.len = 0;
    self->prev_lexem.len = 0;
    self->at_end = false;

    parser_advance(self);
}

void deinit_parser(Parser *self) {
    string_delete(&self->current_lexem);
    string_delete(&self->prev_lexem);
}

ParserState parser_parse_execute(Parser *self) {
    ParserState result = PARSER_SUCCESS;
    char **args = arrinit(char *);

    while (!self->at_end) {
        Token tok = parser_advance(self);
        if (tok.kind == TOKEN_EOL || tok.kind == TOKEN_EOF) {
            arrpush(args, nullptr);
            break;
        }
        char *arg = malloc(tok.lexem_len + 1);
        memcpy(arg, tok.lexem, tok.lexem_len + 1);
        arrpush(args, arg);
    }

    // Check for built-in
    for (size_t i = 0; builtin_str[i] != nullptr; i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            BuiltinStatus res = builtin_func[i](args);
            switch (res) {
            case BUILTIN_FAIL:
                defer(PARSER_FAILURE);
            case BUILTIN_EXIT:
                defer(PARSER_EXIT);
            default:
                defer(PARSER_SUCCESS);
            }
        }
    }

    // Execution
    int status = 0;
    pid_t pid = fork(), wpid;
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("jush");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("jush");
    } else {
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

defer:
    //                                  ٧٧٧-- Ignoring the NULL charcter.
    for (size_t i = 0; i < arrlen(args) - 1; i++) {
        free(args[i]);
    }
    arrfree(args);
    return result;
}

static Token parser_advance(Parser *self) {
    self->prev_lexem.len = self->current_lexem.len;
    memcpy(self->prev_lexem.data, self->current_lexem.data,
           self->current_lexem.len + 1);
    self->current_lexem.len = 0;
    self->current.lexem = self->prev_lexem.data;
    self->prev = self->current;
    for (;;) {
        self->current = scan_token(self);
        if (self->current.kind != TOKEN_ERROR)
            break;

        fprintf(stderr, "Unexpected character '%c'.\n", *self->current_ch);
    }

    return self->prev;
}

static void skip_white_space(Parser *self) {
    char ch = *self->current_ch;

    for (;;) {
        switch (ch) {
        case ' ':
        case '\t':
        case '\r':
            break;
        default:
            return;
        }
        ch = *++self->current_ch;
    }
}

static bool is_forbiden(char ch) {
    switch (ch) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
        return true;
    default:
        return false;
    }
}

static Token scan_name(Parser *self) {
    char ch;
    do {
        ch = *self->current_ch;
        if (ch == '\0') {
            break;
        }

        switch (ch) {
        case '\\':
            string_push(&self->current_lexem, *++self->current_ch);
            break;
        case '"': {
            ch = *++self->current_ch;
            while (ch != '"') {
                string_push(&self->current_lexem, ch);
                ch = *++self->current_ch;
            }
        } break;
        default:
            string_push(&self->current_lexem, ch);
        }

        self->current_ch++;
    } while (!is_forbiden(*self->current_ch));

    return make_token(self, TOKEN_SYMBOL);
}

static Token scan_token(Parser *self) {
    skip_white_space(self);
    self->prev = self->current;

    char ch = *self->current_ch;

    if (ch == '\0') {
        return make_token(self, TOKEN_EOF);
    }

    return scan_name(self);
}

static Token make_token(Parser *self, TokenKind kind) {
    return (Token){
        .lexem = self->current_lexem.data,
        .lexem_len = (size_t)(self->current_ch - self->start),
        .kind = kind,
    };
}
