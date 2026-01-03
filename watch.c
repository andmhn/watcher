#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>

static int fd = 0;
static const int debounce_ms = 100;
char buf[BUFSIZ] __attribute__((aligned(__alignof__(struct inotify_event))));

void watch_init(const char *file) {
    fd = inotify_init1(IN_CLOEXEC);
    if (fd < 0) {
        perror("inotify_init");
        exit(1);
    }

    int wd = inotify_add_watch(fd, file, IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO);

    if (wd < 0) {
        perror("inotify_add_watch");
        exit(1);
    }
}

void watch_once() {
    struct pollfd pfd = {.fd = fd, .events = POLLIN};

    ssize_t len = read(fd, buf, sizeof buf);
    if (len <= 0)
        return;

    while (poll(&pfd, 1, debounce_ms) > 0) {
        // Drain the buffer
        if (read(fd, buf, sizeof buf) <= 0 && errno != EAGAIN) {
            break;
        }
    }
}
