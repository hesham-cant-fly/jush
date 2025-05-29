#include "parser.h"
#include "my_array.h"
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

Parser init_parser(const char *source) {
    Parser parser;
    memset(&parser, 0, sizeof(0));
    parser.source = source;
    parser.start = (char *)source;
    parser.current_ch = (char *)source;
    parser.current_lexem = string_new(30);
    parser.prev_lexem = string_new(30);
    parser.at_end = false;

    parser_advance(&parser);

    return parser;
}

void deinit_parser(Parser *self) { string_delete(&self->current_lexem); }

ParserState parser_parse_execute(Parser *self) {
    char **args = arrinit(char *);

    while (!self->at_end) {
        Token tok = parser_advance(self);
        if (tok.kind == TOKEN_EOL) {
            arrpush(args, NULL);
            break;
        }
        char *arg = malloc(tok.lexem_len + 1);
        memcpy(arg, tok.lexem, tok.lexem_len + 1);
        arrpush(args, arg);
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

    //                                  ٧٧٧-- Ignoring the NULL charcter.
    for (size_t i = 0; i < arrlen(args) - 1; i++) {
        free(args[i]);
    }
    arrfree(args);
    return PARSER_SUCCESS;
}

static Token parser_advance(Parser *self) {
    self->prev_lexem.len = 0;
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
            return make_token(self, TOKEN_EOL);
        }

        switch (ch) {
        case '\\':
            string_push(&self->current_lexem, *++self->current_ch);
            break;
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
        return make_token(self, TOKEN_EOL);
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
