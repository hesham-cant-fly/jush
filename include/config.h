#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define JUSH_DEFAULT_PROMPET " >"
#define JUSH_DEFAULT_VERSION "0.0.1"
#define JUSH_DEFAULT_LEN 50

int print_default_prompet() {
    char *buf = getcwd(nullptr, 0);
    if (buf == nullptr) {
        return -1;
    }
    printf("%s%s ", buf, JUSH_DEFAULT_PROMPET);
    free(buf);
    return 0;
}

#endif // CONFIG_H_
