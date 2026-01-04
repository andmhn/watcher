#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

static int fd = 0;
static const int debounce_ms = 100;
static char buf[BUFSIZ] __attribute__((aligned(__alignof__(struct inotify_event))));

void watch_init(const char *file) {
    fd = inotify_init1(IN_CLOEXEC);
    if (fd < 0) {
        perror("inotify_init");
        exit(1);
    }

    int wd = inotify_add_watch(fd, file, IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO);

    if (wd < 0) {
        fprintf(stderr, "Cannot watch '%s': %s\n", file, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void watch_once() {
    struct pollfd pfd = {.fd = fd, .events = POLLIN};

    ssize_t len = read(fd, buf, sizeof buf);
    if (len <= 0)
        return;

    puts(buf);
    fflush(stdout);
    while (poll(&pfd, 1, debounce_ms) > 0) {
        // Drain the buffer
        len = read(fd, buf, sizeof buf);
        if (len <= 0 && errno != EAGAIN) {
            break;
        }
    }
}

void watch_close() { close(fd); }
