#ifndef _SHELL_H_
#define _SHELL_H_

#include <termios.h>
#include <stdbool.h>
#include <sys/types.h>

/* Whether the shell is connected to an actual terminal or not. */
extern bool shell_is_interactive;

/* File descriptor for the shell input */
extern int shell_terminal;

/* Terminal mode settings for the shell */
extern struct termios shell_tmodes;

/* Process group id for the shell */
extern pid_t shell_pgid;

int shell(int argc, char *argv[]);

#endif
