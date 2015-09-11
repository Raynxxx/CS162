/*
    output redirection with dup2()
    send the output of a command to a file of the user's choice.

    Paul Krzyzanowski
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>

void runcmd(int fd, char **cmd);

int
main(int argc, char **argv)
{
    int newfd;  /* new file descriptor */
    char *cmd[] = { "/bin/ls", "-al", "/", 0 };

    if (argc != 2) {
        fprintf(stderr, "usage: %s output_file\n", argv[0]);
        exit(1);
    }
    if ((newfd = open(argv[1], O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
        perror(argv[1]);    /* open failed */
        exit(1);
    }
    printf("writing output of the command %s to \"%s\"\n", cmd[0], argv[1]);

    runcmd(newfd, cmd); /* run the command, sending the std output to newfd */

    printf("all done!\n");
    exit(0);
}

/*
    runcmd(fd, cmd): fork a child process and run the command cmd,
    sending the standard output to the file descriptor fd.
    The parent waits for the child to terminate.
*/
void
runcmd(int fd, char **cmd)
{
    int status;

    switch (fork()) {
    case 0: /* child */
        dup2(fd, 1);    /* fd becomes the standard output */
        execvp(cmd[0], cmd);
        perror(cmd[0]);     /* execvp failed */
        exit(1);

    default: /* parent */
        while (wait(&status) != -1) ;   /* pick up dead children */
        break;

    case -1: /* error */
        perror("fork");
    }
    return;
}
