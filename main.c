#include "config.h"
#include "my_helpers.h"
#include "parser.h"
#include <readline/history.h>
#include <readline/readline.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

char *read_line() {
    char *prompet = get_default_prompet();
    if (prompet == nullptr)
        return nullptr;
    char *result = readline(prompet);
    if (result == nullptr) {
        free(prompet);
        return nullptr;
    }
    if (result[0] != ' ' && result[0] != '\0')
        add_history(result);
    free(prompet);
    return result;
}

int start_shell() {
    size_t result = 0;
    char *buf = nullptr;
    Parser parser;
    init_parser(&parser);

    while (true) {
        buf = read_line();
        if (buf == nullptr)
            defer(1);
        reinit_parser(&parser, buf);
        ParserState state = parser_parse_execute(&parser);
        switch (state) {
        case PARSER_EXIT:
            defer(0);
        case PARSER_FAILURE:
            defer(1);
        case PARSER_SUCCESS:
            break;
        }
        free(buf);
    }

defer:
    deinit_parser(&parser);
    free(buf);
    return result;
}

int main() {
    int status = start_shell();
    return status;
}
