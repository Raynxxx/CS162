/* This program opens a pipe and creates a copy of itself.  The parent
closes the "read" end of the pipe and writes a couple of lines of text
to the "write" end.  The child closes the "write" end of the pipe,
treats the "read" end as stdin, and replaces itself with the "more"
program. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

typedef int fd; /* low-level Unix file descriptor */
typedef fd pipe_t[2]; /* a two-element array of fd's: one for input, one for output */
const int LINES=1000;

int main (int argc, char **argv) {
	int child_id;
	pipe_t mypipe;

	fprintf (stderr, "Here I am in the program!\n");

	if (pipe (mypipe) < 0) {
		perror ("Pipe creation");
		exit (errno);
		}
	fprintf (stderr, "Got pipe: fd's are %d (read) and %d (write)\n", mypipe[0], mypipe[1]);

	child_id = fork();

	if (child_id) {
		FILE *outfile;
		int counter;
		int status;
		pid_t whodied;

		fprintf (stderr, "I'm the parent.\n");
		if (close (mypipe[0]) < 0) {
			perror ("close read end");
			exit (errno);
			}
		fprintf (stderr, "Successfully closed read end of pipe.\n");

		outfile = fdopen (mypipe[1], "w");
		if (! outfile) {
			perror ("fdopen");
			exit (errno);
			}
		fprintf (stderr, "Successfully opened write end as a FILE *.\n");

		for (counter=0; counter<LINES; ++counter) {
			fprintf (outfile, "Line %d\n", counter);
			}
		/* Another way to write to the  pipe... */
		write (mypipe[1], "Isn't this exciting?\n", 21);

		fprintf (stderr, "Finished writing to file.\n");

		fclose (outfile);
		/* necessary or the child will never get an EOF,
		so it'll never know it's time to die. */

		fprintf (stderr, "Closed file.\n");

		whodied = wait (&status);
		fprintf (stderr, "Parent: Child %d exited ", whodied);
		if (! WIFEXITED(status)) {
			fprintf (stderr, "abnormally!\n");
			}
		else {
			fprintf (stderr, "with status %d.\n", WEXITSTATUS(status));
			}
		return 0;
		}
	else {
		fprintf (stderr, "I'm the child.\n");
		if (close (mypipe[1]) < 0) {
			perror ("close write end");
			exit (errno);
			}
		fprintf (stderr, "Successfully closed write end of pipe\n");

		/* use the "read" end of the pipe as stdin */
		if (dup2 (mypipe[0], 0) < 0) {
			perror ("dup2 read end");
			exit (errno);
			}
		fprintf (stderr, "Successfully dup2'ed read end to stdin.\n");

		execlp ("more", "more", NULL);

		fprintf (stderr, "This statement is never reached.\n");
		}
	}
