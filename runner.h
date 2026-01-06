#ifndef RUNNER_H
#define RUNNER_H

typedef void (*Output_Handler)(char *buf, long n);

void run_cmd_interactive(char *cmds[], Output_Handler);

#endif
