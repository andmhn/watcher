#!/usr/bin/sh

set -x
gcc -o watcher main.c watch.c terminal.c runner.c -Wall -Wextra -O1 -g -fsanitize=address
