#include <errno.h>
#include <pty.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

#include "runner.h"

void run_cmd_interactive(char *cmds[], Output_Handler output_handler) {
    int master_fd;
    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);

    if (pid < 0) {
        perror("forkpty");
        _exit(123);
    }

    if (pid == 0) {
        execvp(cmds[0], cmds);
        perror("execvp");
        _exit(1);
    } else {
        char buffer[4096];
        ssize_t n;
        fd_set read_fds;

        while (1) {
            FD_ZERO(&read_fds);
            FD_SET(STDIN_FILENO, &read_fds);
            FD_SET(master_fd, &read_fds);

            if (select(master_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
                if (errno == EINTR)
                    continue;
                break;
            }

            // Output to screen
            if (FD_ISSET(master_fd, &read_fds)) {
                n = read(master_fd, buffer, sizeof(buffer));
                if (n <= 0)
                    break; // Child exited
                buffer[n] = '\0';
                output_handler(buffer, n);
            }

            // Input to child
            if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                n = read(STDIN_FILENO, buffer, sizeof(buffer));
                if (n <= 0)
                    break;
                n = write(master_fd, buffer, n);
            }
        }

        close(master_fd);
        int status;
        waitpid(pid, &status, 0);
    }
}
