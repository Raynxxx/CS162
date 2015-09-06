#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_ptr_val (int *ptr)
{
    printf ("int value %d\n", *ptr);
}

void fork_print_process ()
{
    pid_t pid = fork();

    if (pid == 0) 
        printf ("I'm child process\n");
    else 
        printf ("I'm parent process\n");
}

int main () {
    int *ptr = NULL;
    ptr = (int *) malloc(sizeof(int));
    *ptr = 42;
    print_ptr_val (ptr);
    free(ptr);
    ptr = NULL;
    //print_ptr_val (ptr);
    //fork_print_process ();
    return 0;
}
