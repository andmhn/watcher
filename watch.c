#define _XOPEN_SOURCE 500
#include <errno.h>
#include <ftw.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

static int fd = 0;
static char buf[BUFSIZ] __attribute__((aligned(__alignof__(struct inotify_event))));

static const int debounce_ms = 100;
static const uint32_t imask  = IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO;

static void add_subdirs(const char *root_path);

void watch_init(const char *dir) {
    fd = inotify_init1(IN_CLOEXEC);
    if (fd < 0) {
        perror("inotify_init");
        exit(1);
    }

    int wd = inotify_add_watch(fd, dir, imask);

    if (wd < 0) {
        fprintf(stderr, "Cannot watch '%s': %s\n", dir, strerror(errno));
        sleep(1);
        exit(EXIT_FAILURE);
    }
    add_subdirs(dir);
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

static int crawl_callback(const char *fpath, const struct stat *sb, int typeflag,
                          struct FTW *ftwbuf) {
    (void) ftwbuf;
    (void) sb;
    if (typeflag == FTW_D) {
        int wd = inotify_add_watch(fd, fpath, imask);
        if (wd == -1) {
            perror("inotify_add_watch");
            return 0; // continue
        }
    }
    return 0; /* To tell nftw() to continue */
}

static void add_subdirs(const char *root_path) {
    const int depth = 20;
    nftw(root_path, crawl_callback, depth, FTW_PHYS);
}

