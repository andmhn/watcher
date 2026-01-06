#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

static struct termios orig_termios;

void clr_scr() { printf("\033[2J\033[H"); }

void terminal_restore() {
    // Leaves Alternate Buffer
    printf("\033[?1049l\033[?25h");
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void terminal_setup() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(terminal_restore);

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    // Enters alternate buffer
    printf("\033[?1049h\033[?25h\033[H");
    fflush(stdout);
}
