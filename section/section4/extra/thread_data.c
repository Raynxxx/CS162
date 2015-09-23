#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */

/* Prototype for thread routine. */
void print_message_function(void *ptr);

/* Struct to hold data to be passed to a thread.
   Demonstrates how multiple datum can be passed to a thread. */
typedef struct thread_data
{
  int thread_no;
  char message[100];
} thread_data_t;

int main()
{
  pthread_t thread1, thread2;  /* thread variables */
  thread_data_t data1, data2;  /* structs to be passed to threads */

  /* Init thread 1 data. */
  data1.thread_no = 1;
  strcpy(data1.message, "Hello! I'm thread 1.");

  /* Init thread 2 data. */
  data2.thread_no = 2;
  strcpy(data2.message, "Hi! I'm thread 2.");

  /* Create threads 1 and 2. */
  pthread_create(&thread1, NULL, (void *)&print_message_function, (void *)&data1);
  pthread_create(&thread2, NULL, (void *)&print_message_function, (void *)&data2);

  /* Main thread now waits for both spawned threads to terminate, before it
   * exits. If main block exits first, both threads exit, even if the threads
   * have not finished their work. */
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);

  /* exit */
  exit(0);
} /* main() */

/**
 * print_message_function is used as the start routine for the threads used
 * it accepts a void pointer
**/
void print_message_function ( void *ptr )
{
  thread_data_t *data;
  data = (thread_data_t *) ptr;  /* type cast to a pointer to thread_data_t */

  printf("Thread %d says: %s \n", data->thread_no, data->message);

  pthread_exit(0); /* exit */
}
