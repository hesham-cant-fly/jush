#ifndef TOKENIZER_H_
#define TOKENIZER_H_

#include "environment.h"
#include "my_string.h"
#include <stddef.h>
#include <stdint.h>

typedef enum TokenKind : uint8_t {
    TOKEN_EOL = 0,
    TOKEN_EOF = 1,
    TOKEN_ERROR,
    TOKEN_SYMBOL,
    TOKEN_AMPERSAND, // '&'
    TOKEN_SEMICOLON, // ';'
} TokenKind;

typedef struct Token {
    /* size_t line; */
    /* size_t column; */
    size_t lexem_len;
    TokenKind kind;
    const char *lexem;
} Token;

typedef struct Tokenizer {
    Environment *env;
    Token current_tok;
    Token prev_tok;
    String current_lexem;
    String prev_lexem;
    size_t start;
    size_t current;
    char *source;
    bool done;
} Tokenizer;

Tokenizer init_tokenizer(char *source, Environment *env);
void deinit_tokenizer(Tokenizer *self);
Token scan_token(Tokenizer *self);

#endif // TOKENIZER_H_
