#ifndef WATCH_H
#define WATCH_H

enum Event {
    Console_Input,
    File_Update,
};

typedef void (*EventHandler)(enum Event events[], int n_events, void *userdata);

void watch_init(const char **dirs, size_t n_dir);
void watch_once(EventHandler event_callback, void *userdata);
void watch_close();

#endif
