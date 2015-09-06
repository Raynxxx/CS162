#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#include <stdlib.h>
 
#define OUTPATH "output"
#define MESSAGE "Behold, Standard Out is now a file!"
 
int main() {
    /*First, we open a file for writing only"*/
    int outputfd = -1;
 
    outputfd = open(OUTPATH, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRWXU | S_IRGRP | S_IROTH);
 
    /*If we have an error, we exit
    * N.B. file descriptors less than one are invalid*/
    if (outputfd < 0) {
        perror("open(2) file: " OUTPATH);
        exit(EXIT_FAILURE);
    }
 
    /*Next, we close Standard Out
    The lowest file descriptor will now
    be STDOUT_FILENO*/
    if (close(STDOUT_FILENO) < 0) {
        perror("close(2) file: STDOUT_FILENO");
        close(outputfd);
        exit(EXIT_FAILURE);
    }
 
    /*Afterwards, we duplicate outputfd onto STDOUT_FILENO,
    exiting if the descriptor isn't equal to STDOUT_FILENO*/
    if (dup(outputfd) != STDOUT_FILENO) {
        perror("dup(2)");
        close(outputfd); /*N.B. Remember to close your files!*/
        exit(EXIT_FAILURE);
    }
 
    close(outputfd); /*If everything succeeds, we may close the original file*/
    puts(MESSAGE); /*and then write our message*/
 
    return EXIT_SUCCESS;
}