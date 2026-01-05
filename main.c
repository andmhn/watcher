#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include "watch.h"

void clr_scr();
void terminal_setup();
void terminal_restore();
void handle_exit(int sig);

typedef struct {
    char *dir;
    char **cmds;
    size_t n_cmd;
} Context;

void run(char *cmds[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmds[0], &cmds[0]);
        perror("execvp");
        _exit(127);
    }
    if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
}

void handle_file_update(Context *ctx) {
    clr_scr();

    printf("watching in  %s\n", ctx->dir);
    printf("running: ");
    for (size_t i = 0; i < ctx->n_cmd; i++)
        printf("%s ", ctx->cmds[i]);
    printf("\n\n");

    run(ctx->cmds);
    fflush(stdout);
}

void handle_console_input(Context *ctx) {
    char buf;
    if (read(STDIN_FILENO, &buf, 1) > 0) {
        switch (buf) {

        case 'q':
            handle_exit(0);
            break;

        case 'r':
            handle_file_update(ctx);
            break;
        }
    }
}

void event_handler(enum Event events[], int nevents, void *ctx) {
    for (int i = 0; i < nevents; i++) {
        switch (events[i]) {
        case Console_Input: {
            handle_console_input(ctx);
            break;
        }
        case File_Update: {
            handle_file_update(ctx);
            break;
        }
        }
    }
}

void watch_and_run(Context ctx) {
    handle_file_update(&ctx); // initial run
    while (1) {
        watch_init(ctx.dir);
        watch_once(event_handler, &ctx);
        watch_close();
    }
}

void print_help() {
    puts("\nUSAGE:\twatcher <dir> <cmds...>\n\n");
    puts("\tWatch for file changes in directory and run comand\n");
}

void handle_exit(int sig) {
    (void)sig;
    terminal_restore();
    fflush(stdout);
    _exit(sig);
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        print_help();
        return 1;
    }

    char *dir = argv[1];

    terminal_setup();
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);

    Context ctx = {
        .dir = dir,
        .cmds = &argv[2],
        .n_cmd = argc - 2,
    };

    watch_and_run(ctx);
}
