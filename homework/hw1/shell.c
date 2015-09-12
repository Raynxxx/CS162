#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>

#include "io.h"
#include "parse.h"
#include "process.h"
#include "shell.h"

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_quit(tok_t arg[]);
int cmd_help(tok_t arg[]);
int cmd_pwd(tok_t arg[]);
int cmd_cd(tok_t arg[]);
int cmd_wait(tok_t arg[]);

char* get_current_time();
char* find_file_from_path(char* filename, tok_t path_tokens[]);
int io_redirect(tok_t arg[]);
void path_resolve(tok_t arg[], tok_t path_tokens[]);
void undo_signal();

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(tok_t args[]);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_quit, "quit", "quit the command shell"},
  {cmd_pwd, "pwd", "show the current working directory"},
  {cmd_cd, "cd", "changes the current working directory"},
  {cmd_wait, "wait", "wait until all background jobs have terminated"}
};

/**
 * Prints a helpful description for the given command
 */
int cmd_help(tok_t arg[]) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  }
  return 1;
}

/**
 * Quits this shell
 */
int cmd_quit(tok_t arg[]) {
  exit(0);
  return 1;
}

/**
 * Prints the current working directory
 */
int cmd_pwd(tok_t arg[]) {
  char* cwd = (char*) malloc(PATH_MAX + 1);
  getcwd(cwd, PATH_MAX);
  if (cwd != NULL) {
    printf("%s\n", cwd);
    free(cwd);
    return 0;
  }
  return 1;
}

/**
 * Changes the current working directory to a specified directory
 */
int cmd_cd(tok_t arg[]) {
  if (chdir(arg[0]) == -1) {
    printf("%s : %s\n", arg[0], strerror(errno));
    return 1;
  }
  return 0;
}

/**
 * Waits until all background jobs have terminated
 */
int cmd_wait(tok_t arg[]) {
  int status;
  pid_t pid;
  while ((pid = waitpid(-1, &status, 0)) > 0) {
    if (WIFEXITED(status)) {
      printf("child [%d] has terminted with exit status = %d\n", pid, WEXITSTATUS(status));
    } else {
      printf("child [%d] terminated with error\n", pid);
    }
  }
  if (errno != ECHILD) {
    fprintf(stderr, "waitpid error\n");
  }
  return 0;
}


/**
 * Get current time
 */
char* get_current_time() {
    static char time_buf[64];
    time_t raw_time;
    struct tm* cur_time;

    time(&raw_time);
    cur_time = localtime(&raw_time);
    strftime (time_buf, sizeof(time_buf), "%H:%M:%S", cur_time);
    return time_buf;
}

/**
 * Find execuatable file from PATH.
 */
char* find_file_from_path(char* filename, tok_t path_tokens[]) {
  char* ret = (char*) malloc(PATH_MAX + MAXLINE + 2);
  struct dirent* ent;
  for (int i = 1; i < MAXTOKS && path_tokens[i]; ++i) {
    DIR* dir;
    if ((dir = opendir(path_tokens[i])) == NULL)
      continue;
    while ((ent = readdir(dir)) != NULL) {
      if (strcmp(ent->d_name, filename) == 0) {
        strncpy(ret, path_tokens[i], PATH_MAX);
        strncat(ret, "/", 1);
        strncat(ret, filename, MAXLINE);
        return ret;
      }
    }
    closedir(dir);
  }
  free(ret);
  return NULL;
}

/**
 * IO Redirection
 */
int io_redirect(tok_t arg[]) {
  int i, fd;
  int in_redir = is_direct_tok(arg, "<");
  int out_redir = is_direct_tok(arg, ">");
  if (in_redir != 0) {
    if (arg[in_redir + 1] == NULL ||
        strcmp(arg[in_redir + 1], ">") == 0 ||
        strcmp(arg[in_redir + 1], "<") == 0)
    {
      fprintf(stderr, "%s : Syntax error.\n", arg[0]);
      return -1;
    }
    if ((fd = open(arg[in_redir + 1], O_RDONLY, 0)) < 0) {
      fprintf(stderr, "%s : No such file or directory.\n", arg[in_redir + 1]);
      return -1;
    }
    dup2(fd, STDIN_FILENO);
    for (i = in_redir; i < MAXTOKS - 2 && arg[i + 2]; ++i) {
      free(arg[i]);
      arg[i] = arg[i + 2];
    }
    arg[i] = NULL;
  }
  if (out_redir != 0) {
    if (arg[out_redir + 1] == NULL ||
        strcmp(arg[out_redir + 1], ">") == 0 ||
        strcmp(arg[out_redir + 1], "<") == 0)
    {
      fprintf(stderr, "%s : Syntax error.\n", arg[0]);
      return -1;
    }
    if ((fd = open(arg[out_redir + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
      fprintf(stderr, "%s : No such file or directory.\n", arg[out_redir + 1]);
      return -1;
    }
    dup2(fd, STDOUT_FILENO);
    for (i = out_redir; i < MAXTOKS - 2 && arg[i + 2]; ++i) {
      free(arg[i]);
      arg[i] = arg[i + 2];
    }
    arg[i] = NULL;
  }
  return 0;
}

/**
 * Path resolve
 */
void path_resolve(tok_t arg[], tok_t path_tokens[]) {
  char* eval = find_file_from_path(arg[0], path_tokens);
  if (eval != NULL) {
    arg[0] = eval;
  }
}

/**
 * undo the default singal action for child job
 */
void undo_signal() { 
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTSTP, SIG_DFL);
  signal(SIGTTIN, SIG_DFL);
  signal(SIGTTOU, SIG_DFL);
}

/**
 * Looks up the built-in command, if it exists.
 */
int lookup(char cmd[]) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++) {
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0)) return i;
  }
  return -1;
}

/**
 * Intialization procedures for this shell
 */
void init_shell() {
  /* Check if we are running interactively */
  shell_terminal = STDIN_FILENO;
  shell_is_interactive = isatty(shell_terminal);

  if(shell_is_interactive){
    /* Force the shell into foreground */
    while(tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Ignore interactive and job-control signals */
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    //signal(SIGCHLD, SIG_IGN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Put shell in its own process group */
    if (setpgid(shell_pgid, shell_pgid) < 0) {
      fprintf(stderr, "Couldn’t put the shell in its own process group");
      exit(1);
    }

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

int shell(int argc, char *argv[]) {
  char *input_bytes;
  tok_t *tokens;
  tok_t *path_tokens;
  int fundex = -1;
  int tokens_length = 0;
  char* path = (char*) malloc(PATH_MAX + 1);
  char* cwd_buf = (char*) malloc(PATH_MAX + 1);

  /* copy a new path var to use */
  strncpy(path, getenv("PATH"), PATH_MAX);
  path_tokens = get_toks(path);


  init_shell();

  if (shell_is_interactive) {
    getcwd(cwd_buf, PATH_MAX);
    /* Please only print shell prompts when standard input is not a tty */
    fprintf(stdout, "\033[;34mrayn\33[0m [\033[;32m%s\33[0m] \033[;31m%s\33[0m > ", get_current_time(), cwd_buf);
  }

  while ((input_bytes = freadln(stdin))) {
    tokens = get_toks(input_bytes);
    tokens_length = toks_length(tokens);
    int bg = 0;
    if (strcmp(tokens[tokens_length - 1], "&") == 0) {
      bg = 1;
      free(tokens[tokens_length - 1]);
      tokens[tokens_length - 1] = NULL;
    }
    fundex = lookup(tokens[0]);
    if (fundex >= 0) {
      cmd_table[fundex].fun(&tokens[1]);
    } else {
      /* REPLACE this to run commands as programs. */
      pid_t pid = fork();
      if (pid == 0) {
        if (io_redirect(tokens) < 0) {
          exit(0);
        }
        path_resolve(tokens, path_tokens);
        undo_signal();

        if (execv(tokens[0], tokens) < 0) {
          fprintf(stderr, "%s : Command not found\n", tokens[0]);
          exit(0);
        }
      } else {
        if (!bg) {
          int child_status;
          if (waitpid(pid, &child_status, 0) < 0) {
              fprintf(stderr, "waitpid error\n");
          }
        } else {
          printf("[%d] : %s\n", pid, input_bytes);
        }
      }
    }
    free_toks(tokens);
    free(input_bytes);

    if (shell_is_interactive) {
      getcwd(cwd_buf, PATH_MAX);
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "\033[;34mrayn\33[0m [\033[;32m%s\33[0m] \033[;31m%s\33[0m > ", get_current_time(), cwd_buf);
    }
  }
  free(path);
  free_toks(path_tokens);
  free(cwd_buf);
  return 0;
}
