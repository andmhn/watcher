#!/usr/bin/sh

gcc main.c watch.c terminal.c -Wall -Wextra -g -O1 -fsanitize=address -o watcher
