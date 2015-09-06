/* signal example #1: trap an illegal address reference */

#include <stdio.h>	/* for printf */
#include <stdlib.h>	/* for exit */
#include <signal.h>	/* defines signals and the signal() function */

main(int argc, char **argv) {
	void catch(int);
	int a;

	signal(SIGSEGV, catch);	/* catch SIGSEGV - segmentation violation */
	a = *(int *)0;	/* read address 0 */
}

void
catch(int snum) {
	printf("got signal %d\n", snum);
	exit(0);
}
