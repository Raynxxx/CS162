/* This program takes in an argument, a filename.
It opens that file for reading, sets stdin to refer to that file,
creates a copy of itself, the child switches to the "cat" program;
the parent waits for the child to die. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

int main (int argc, char **argv) {
	int child_id;
	char *filename;
	int status;
	pid_t whodied;
	FILE *infile;

	if (argc == 1)
		infile = stdin;
	else {
		infile = freopen (argv[1], "r", stdin); //maps contents of argv[1] to stdin stream
		if (! infile) {
			fprintf (stderr, "Couldn't open file %s!\n", argv[1]);
			perror ("example 4");
			exit (errno);
			}
		}

	printf ("I opened the file successfully!\n");

	child_id = fork();

	if (child_id) {
		printf ("I'm the parent.\n");
		whodied = wait (&status);
		printf ("Parent: Child %d exited ", whodied);
		if (! WIFEXITED(status)) { //returns true if the child terminated normally //pkill -TERM -P pid
			printf ("abnormally!\n");
			}
		else {
			printf ("with status %d.\n", WEXITSTATUS(status));
			}
		return 0;
		}
	else {
		printf ("I'm the child.\n");
		execlp ("cat", "cat", NULL);
		printf ("This statement is never reached.\n");
		}
	}
