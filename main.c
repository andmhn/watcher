#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "runner.h"
#include "watch.h"

void clr_scr();
void terminal_setup();
void terminal_restore();
void handle_exit(int sig);

typedef struct {
    char **dirs;
    size_t n_dir;
    char **cmds;
    size_t n_cmd;
} Context;

void cmd_output_callback(char *buffer, long n) {
    (void)n;
    printf("%s", buffer);
    fflush(stdout);
}

void handle_file_update(Context *ctx) {
    clr_scr();

    printf("watching in ");
    for (size_t i = 0; i < ctx->n_dir; i++)
        printf("%s ", ctx->dirs[i]);
    printf("\n");

    printf("running: ");
    for (size_t i = 0; i < ctx->n_cmd; i++)
        printf("%s ", ctx->cmds[i]);
    printf("\n\n");

    run_cmd_interactive(ctx->cmds, cmd_output_callback);
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

void event_handler(enum Event events[], int n_events, void *ctx) {
    for (int i = 0; i < n_events; i++) {
        switch (events[i]) {
        case Console_Input:
            handle_console_input(ctx);
            break;

        case File_Update:
            handle_file_update(ctx);
            break;
        }
    }
}

void watch_and_run(Context ctx) {
    handle_file_update(&ctx); // initial run
    while (1) {
        watch_init((const char **)ctx.dirs, ctx.n_dir);
        watch_once(event_handler, &ctx);
        watch_close();
    }
}

void print_help() {
    puts("\nUSAGE:\twatcher <dirs..> -- <cmds...>\n\n");
    puts("\tWatch for file changes in directory and run comand\n");
}

void handle_exit(int sig) {
    (void)sig;
    terminal_restore();
    fflush(stdout);
    _exit(sig);
}

Context parse_args(int argc, char *argv[]) {
    int i = 1;
    while(i < argc) {
        if(strncmp(argv[i], "--", 2) == 0)
            break;
        i++;
    }
    if(i == argc) {
        print_help();
        handle_exit(1);
    }

    int n_dir = i - 1;
    int cmd_i = i + 1;

    return (Context) {
        .dirs  = &argv[1],
        .n_dir = n_dir,
        .cmds  = &argv[cmd_i],
        .n_cmd = argc - cmd_i,
    };
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        print_help();
        return 1;
    }

    terminal_setup();
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);

    Context ctx = parse_args(argc, argv);

    watch_and_run(ctx);
}
