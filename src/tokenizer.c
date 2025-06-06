#include "tokenizer.h"
#include "environment.h"
#include "my_helpers.h"
#include "my_string.h"
#include <readline/readline.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef Tokenizer *Self;

static char peek(Self self);
static char advance(Self self);
static char previous(Self self);
static size_t lexem_len(Self self);
static bool is_forbiden(char ch);
static bool _isalpha(const char ch);
static bool _isnum(const char ch);
static bool _isalnum(const char ch);
static Token scan_substitution(Self self);
static Token scan_symbol(Self self);
static void skip_white_space(Self self);
static void setup_scan(Self self);
static Token make_token(Self self, TokenKind kind);

Tokenizer init_tokenizer(char *source, Environment *env) {
    Tokenizer tok;
    memset(&tok, 0, sizeof(Tokenizer));
    tok.env = env;
    tok.current = 0;
    tok.start = 0;
    tok.source = source;
    tok.current_lexem = string_new(30);
    tok.prev_lexem = string_new(30);
    tok.done = false;
    return tok;
}

void deinit_tokenizer(Tokenizer *self) {
    string_delete(&self->prev_lexem);
    string_delete(&self->current_lexem);
}

Token scan_token(Tokenizer *self) {
    setup_scan(self);
    skip_white_space(self);

    char ch = peek(self);
    if (ch == '\0') {
        self->done = true;
        return make_token(self, TOKEN_EOF);
    }

    advance(self);
    switch (ch) {
    case ';':
        return make_token(self, TOKEN_SEMICOLON);
    case '\n':
        return make_token(self, TOKEN_EOL);
    case '&':
        return make_token(self, TOKEN_AMPERSAND);
    case '$':
        return scan_substitution(self);
    case '~':
        string_push(&self->current_lexem, getenv("HOME"));
        return make_token(self, TOKEN_SYMBOL);
    default:
        self->current--;
        return scan_symbol(self);
    }
}

static Token scan_symbol(Self self) {
    char ch;
    do {
        ch = peek(self);
        if (ch == '\0') {
            if (lexem_len(self) == 0)
                return make_token(self, TOKEN_EOF);
            break;
        }

        switch (ch) {
        case '\\':
            advance(self);
            string_push(&self->current_lexem, peek(self));
            break;
        case '"':
            advance(self);
            while (peek(self) != '"') {
                char ch = advance(self);
                switch (ch) {
                case '$':
                    scan_substitution(self);
                    break;
                default:
                    string_push(&self->current_lexem, ch);
                }
            }
            break;
        default:
            string_push(&self->current_lexem, ch);
        }

        advance(self);
        ch = peek(self);
    } while (!is_forbiden(ch));

    return make_token(self, TOKEN_SYMBOL);
}

static Token scan_substitution(Self self) {
    // TODO: Add support for getting arguments
    char ch = peek(self);
    switch (ch) {
    case '!': {
        advance(self);
        char *buf = (char *)env_get(self->env, "!");
        if (buf == nullptr)
            buf = "";
        string_push(&self->current_lexem, buf);
        return make_token(self, TOKEN_SYMBOL);
    }
    case '?': {
        advance(self);
        char *buf = (char *)env_get(self->env, "?");
        if (buf == nullptr)
            buf = "0";
        string_push(&self->current_lexem, buf);
        return make_token(self, TOKEN_SYMBOL);
    }
    default: {
        size_t current_lexem_len = self->current_lexem.len;

        if (!_isalpha(peek(self))) {
            string_push(&self->current_lexem, previous(self));
            Token res = make_token(self, TOKEN_SYMBOL);
            return res;
        }

        size_t varname_len = 0;
        while (_isalnum(peek(self))) {
            string_push(&self->current_lexem, advance(self));
            varname_len++;
        }
        const char *sym =
            env_get(self->env, self->current_lexem.data + current_lexem_len);
        if (sym == nullptr)
            sym = "";
        self->current_lexem.len -= varname_len;
        string_push(&self->current_lexem, (char *)sym);
        return make_token(self, TOKEN_SYMBOL);
    }
    }
    unreachable();
}

static char peek(Self self) { return self->source[self->current]; }
static char advance(Self self) { return self->source[self->current++]; }
static char previous(Self self) { return self->source[self->current - 1]; }
static size_t lexem_len(Self self) { return self->current - self->start; }
static bool is_forbiden(char ch) {
    switch (ch) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case ';':
        return true;
    default:
        return false;
    }
}
static bool _isalpha(const char ch) {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}
static bool _isnum(const char ch) { return ch >= '0' && ch <= '9'; }
static bool _isalnum(const char ch) { return _isalpha(ch) || _isnum(ch); }

static void skip_white_space(Self self) {
    char ch = peek(self);

    loop {
        switch (ch) {
        case ' ':
        case '\t':
        case '\r':
            break;
        default:
            return;
        }
        advance(self);
        ch = peek(self);
    }
}

static void setup_scan(Self self) {
    self->start = self->current;

    self->prev_lexem.len = 0;
    string_push(&self->prev_lexem, &self->current_lexem);
    self->current_lexem.len = 0;

    self->current_tok.lexem = self->prev_lexem.data;
    self->prev_tok = self->current_tok;
}

static Token make_token(Self self, TokenKind kind) {
    return (Token){
        .lexem = self->current_lexem.data,
        .lexem_len = (size_t)(self->current - self->start),
        .kind = kind,
    };
}
