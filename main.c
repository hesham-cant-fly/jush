#include "config.h"
#include "parser.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int read_line(char **out, size_t *len) {
    int status = getline(out, len, stdin);
    return status;
}

int start_shell() {
    int status = 0;
    char *b = malloc(JUSH_DEFAULT_LEN + 1);
    size_t input_len = JUSH_DEFAULT_LEN;
    while (true) {
        status = print_default_prompet();
        if (status == -1) {
            free(b);
            return 1;
        }
        status = read_line(&b, &input_len);
        if (status == -1) {
            free(b);
            return 1;
        }
        Parser p = init_parser(b);
        parser_parse_execute(&p);
        deinit_parser(&p);
    }
    free(b);
    return 0;
}

int main() {
    int status = start_shell();
    return status;
}
