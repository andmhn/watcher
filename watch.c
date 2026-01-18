#define _XOPEN_SOURCE 500
#include <errno.h>
#include <ftw.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include "watch.h"

static struct pollfd fds[2] = {0};
static int fd = 0;
static char buf[BUFSIZ] __attribute__((aligned(__alignof__(struct inotify_event))));

static const int debounce_ms = 100;
static const uint32_t imask = IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_TO;

static void add_subdirs(const char *root_path);
static void add_dir_to_watch(const char* dir);

void watch_init(const char **dirs, size_t n_dir) {
    fd = inotify_init1(IN_CLOEXEC);
    if (fd < 0) {
        perror("inotify_init");
        exit(1);
    }

    for(size_t  i = 0 ; i < n_dir; i++ ) {
        add_dir_to_watch(dirs[i]);
        add_subdirs(dirs[i]);
    }

    /* Prepare for polling. */

    fds[0].fd = STDIN_FILENO; /* Console input */
    fds[0].events = POLLIN;

    fds[1].fd = fd; /* Inotify input */
    fds[1].events = POLLIN;
}

void watch_once(EventHandler event_callback, void *userdata) {
    size_t n_events = 0;
    enum Event events[2];

    //--------------------------  POLL  ---------------------
    const int n_fds = 2;
    int poll_num = poll(fds, n_fds, -1);
    if (poll_num == -1) {
        if (errno == EINTR)
            return;
        perror("poll");
        exit(EXIT_FAILURE);
    }

    //--------------------------  READ EVENTS ----------------
    if (poll_num > 0) {

        if (fds[0].revents & POLLIN) {
            events[n_events++] = Console_Input;
        }

        if (fds[1].revents & POLLIN) {

            while (poll(&fds[1], 1, debounce_ms) > 0) {
                // Let it drain
                size_t len = read(fd, buf, sizeof(buf));

                if (len <= 0 && errno != EAGAIN)
                    break;
            }
            events[n_events++] = File_Update;
        }
    }

    if (n_events > 0)
        event_callback(events, n_events, userdata);
}

void watch_close() { close(fd); }

static void add_dir_to_watch(const char* dir) {
    int wd = inotify_add_watch(fd, dir, imask);

    if (wd < 0) {
        fprintf(stderr, "Cannot watch '%s': %s\n", dir, strerror(errno));
        sleep(1);
        exit(EXIT_FAILURE);
    }
}

static int crawl_callback(const char *fpath, const struct stat *sb, int typeflag,
                          struct FTW *ftwbuf) {
    (void)ftwbuf;
    (void)sb;
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
