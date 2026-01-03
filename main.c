#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void run(char* cmds[]){
    pid_t pid = fork();
    if (pid == 0) {
        execvp(cmds[0], &cmds[0]);
    }
    waitpid(pid, NULL, 0);
}

void watch_and_run(const char *dir, char *cmds[]) {
    printf("watching in : %s\n", dir);
    printf("running: %s\n\n", cmds[0]);
    
    run(cmds);
    printf("OK\n");
}

void print_help() {
    puts("USAGE:\twatcher <dir> <cmd...>\n\n");
    puts("\tWatch for file changes in directory and run comand");
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        print_help();
        return 1;
    }

    char *dir = argv[1];

    watch_and_run(dir, &argv[2]);
}
