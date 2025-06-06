#include "config.h"
#include "environment.h"
#include "launcher.h"
#include "my_helpers.h"
#include <readline/history.h>
#include <readline/readline.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

void handle_sigchld(int sig) {
    unused(sig);
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {

        if (WIFEXITED(status)) {
            printf("[?] Done! %d", getpid());
            printf("  Exit status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[?] Done! %d", getpid());
            printf("  Terminated by signal: %d\n", WTERMSIG(status));
        } else {
            printf("[?] Done! %d\n", getpid());
        }
    }
}

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

int run_file(const char *path, Environment *env) {
    int result = 0;
    FILE *f = fopen(path, "r");
    if (f == nullptr) {
        return result;
    }
    fseek(f, 0, SEEK_END);
    size_t content_len = ftell(f);
    rewind(f);

    char *content = malloc(content_len + 1);
    if (content == nullptr) {
        defer(1);
    }
    if (content_len != fread(content, sizeof(char), content_len, f)) {
        defer(1);
    }
    content[content_len] = '\0';

    LauncherState state = launch(content, env);
    switch (state) {
    case LAUNCHER_SUCCESS:
        break;
    default:
        result = 1;
    }

defer:
    if (content != nullptr) {
        free(content);
    }
    fclose(f);
    return result;
}

int start_shell() {
    size_t result = 0;
    char *buf = nullptr;
    Environment env = init_env();
    {
        char *path = MOSH_DEFAULT_CONFIG_PATH;
        run_file(path, &env);
        free(path);
    }

    while (true) {
        buf = read_line();
        if (buf == nullptr)
            defer(1);
        LauncherState state = launch(buf, &env);
        switch (state) {
        case LAUNCHER_EXIT:
            defer(0);
        case LAUNCHER_FAILURE:
        case LAUNCHER_SUCCESS:
            break;
        }
        free(buf);
    }

defer:
    free(buf);
    deinit_env(&env);
    return result;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction failed");
        exit(1);
    }

    int status = start_shell();
    return status;
}
