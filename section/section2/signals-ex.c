/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <signal.h>     /* Signal handling */


/* This will be our new SIGINT handler.
   SIGINT is generated when user presses Ctrl-C.
   Normally, program will exit with Ctrl-C.
   With our new handler, it won't exit. */
void mysigint()
{
    printf("I caught the SIGINT signal!\n");
    return;    
} 

/* Our own SIGKILL handler */
void mysigkill()
{
    printf("I caught the SIGKILL signal!\n");
    return;    
} 

/* Our own SIGHUP handler */
void mysighup()
{
    printf("I caught the SIGHUP signal!\n");
    return;    
} 

/* Our own SIGTERM handler */
void mysigterm()
{
    printf("I caught the SIGTERM signal!\n");
    return;    
} 

int main()
{
    /* Use the signal() call to associate our own functions with
       the SIGINT, SIGHUP, and SIGTERM signals */
    if (signal(SIGINT, mysigint) == SIG_ERR)
       printf("Cannot handle SIGINT!\n");        
    if (signal(SIGHUP, mysighup) == SIG_ERR)
       printf("Cannot handle SIGHUP!\n");        
    if (signal(SIGTERM, mysigterm) == SIG_ERR)
       printf("Cannot handle SIGTERM!\n");        
           
    /* can SIGKILL be handled by our own function? */ //kill -9 pid
    if (signal(SIGKILL, mysigkill) == SIG_ERR) 
       printf("Cannot handle SIGKILL!\n");        
    
    printf("My PID is %d.\n", getpid());
    while(1);  /* infinite loop */

    /* exit */  
    exit(0);
} /* main() */
