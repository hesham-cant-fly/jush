#ifndef PARSER_H_
#define PARSER_H_

#include "my_string.h"
#include <stddef.h>
#include <stdint.h>

typedef enum ParserState : int {
    PARSER_SUCCESS,
    PARSER_FAILURE,
} ParserState;

typedef enum TokenKind : uint8_t {
    TOKEN_SYMBOL,
    TOKEN_EOL,
    TOKEN_EOF,
    TOKEN_ERROR,
} TokenKind;

typedef struct Token {
    /* size_t line; */
    /* size_t column; */
    size_t lexem_len;
    TokenKind kind;
    const char *lexem;
} Token;

typedef struct Parser {
    Token current;
    Token prev;
    String prev_lexem;
    String current_lexem;
    char *current_ch;
    char *start;
    const char *source;
    bool at_end;
} Parser;

Parser init_parser(const char *source);
void deinit_parser(Parser *self);
ParserState parser_parse_execute(Parser *self);

#endif // PARSER_H_
