#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

void watch_init(const char *file);
void watch_once();
void watch_close();

void clr_scr();
void terminal_setup();
void terminal_restore();

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

void watch_and_run(const char *dir, char *cmds[]) {
    while (1) {
        watch_init(dir);
        clr_scr();
        printf("watching in : %s\n", dir);
        printf("running: %s\n\n", cmds[0]);

        run(cmds);

        fflush(stdout);
        watch_once();
        watch_close();
    }
}

void print_help() {
    puts("USAGE:\twatcher <dir> <cmd...>\n\n");
    puts("\tWatch for file changes in directory and run comand");
}

void handle_exit(int sig) {
    (void)sig;
    terminal_restore();
    fflush(stdout);
    _exit(-1);
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

    watch_and_run(dir, &argv[2]);
}
