/* signal3: parent creates a child and detects its termination with a signal */
/*  The child prints an "I'm the child message and exits after 3 seconds */

#include <stdlib.h>	/* needed to define exit() */
#include <unistd.h>	/* needed for fork() and getpid() */
#include <signal.h>	/* needed for signal */
#include <stdio.h>	/* needed for printf() */

int
main(int argc, char **argv) {
	void catch(int);	/* signal handler */
	void child(void);	/* the child calls this */
	void parent(int pid);	/* the parent calls this */
	int pid;	/* process ID */

	signal(SIGCHLD, catch);	/* detect child termination */

	switch (pid = fork()) {
	case 0:		/* a fork returns 0 to the child */
		child();
		break;

	default:	/* a fork returns a pid to the parent */
		parent(pid);
		break;

	case -1:	/* something went wrong */
		perror("fork");
		exit(1);
	}
	exit(0);
}

void
child(void) {
	printf("        child: I'm the child\n");
	sleep(3);	/* do nothing for 3 seconds */
	printf("        child: I'm exiting\n");
	exit(123);	/* exit with a return code of 123 */
}

void
parent(int pid) {
	printf("parent: I'm the parent\n");
	sleep(10);	/* do nothing for five seconds */
	printf("parent: exiting\n");
}

void
catch(int snum) {
	int pid;
	int status;

	pid = wait(&status);
	printf("parent: child process exited with value %d\n", WEXITSTATUS(status));
}
