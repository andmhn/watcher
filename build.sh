#!/usr/bin/sh

gcc main.c watch.c -Wall -Wextra -g -O1 -fsanitize=address -o watcher
