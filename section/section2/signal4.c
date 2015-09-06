/* signal4: parent creates multiple child processes and detects
   their termination via a signal 
   Each child prints an "I'm the child" message and exits after n seconds,
   where n is the sequence in which it was forked. 
*/
#include <stdlib.h>	/* needed to define exit() */
#include <unistd.h>	/* needed for fork() and getpid() */
#include <signal.h>	/* needed for signal */
#include <stdio.h>	/* needed for printf() */

#define NUMPROCS 4	/* number of processes to fork */
int nprocs;		/* number of child processes */

int
main(int argc, char **argv) {
	void catch(int);	/* signal handler */
	void child(int n);	/* the child calls this */
	void parent(int pid);	/* the parent calls this */
	int pid;	/* process ID */
	int i;

	signal(SIGCHLD, catch);	/* detect child termination */

	for (i=0; i < NUMPROCS; i++) {
		switch (pid = fork()) {
		case 0:		/* a fork returns 0 to the child */
			child(i);	/* child() never returns; the function exits */
			break;
		case -1:	/* something went wrong */
			perror("fork");
			exit(1);
		default:	/* parent just loops to create more kids */
			nprocs++;	/* count # of processes that we forked */
			break;
		}
	}
	printf("parent: going to sleep\n");

	/* do nothing forever; remember that a signal wakes us out of sleeep */
	while (nprocs != 0) {
		printf("parent: sleeping\n");
		sleep(60);	/* do nothing for a minute */
	}
	printf("parent: exiting\n");
	exit(0);
}

void
child(int n) {
	printf("\tchild[%d]: child pid=%d, sleeping for %d seconds\n", n, getpid(), n);
	sleep(n);	/* do nothing for n seconds */
	printf("\tchild[%d]: I'm exiting\n", n);
	exit(100+n);	/* exit with a return code of 100+n */
}

void
catch(int snum) {
	int pid;
	int status;

	pid = wait(&status);
	printf("parent: child process pid=%d exited with value %d\n",
		pid, WEXITSTATUS(status));
	nprocs--;
	signal(SIGCHLD, catch);	/* reset the siganl (for SunOS) */
}