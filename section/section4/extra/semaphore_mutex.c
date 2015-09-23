//gcc sem-ex2.c -Wall -pedantic -std=c99 -D_XOPEN_SOURCE=700 -pthread
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <semaphore.h>  /* Semaphore */

#define THREAD_SLEEP_SEC 1
#define SEMAPHORE_VAL 3
#define NTHREADS 10

/* Prototype for thread routine. */
void handler(void *ptr);

/* Semaphores are declared globally so they can be accessed in main() and in
 * thread routines. Here, the semaphore is used as a sem (aka, a LOCK). */
sem_t sem;
int counter; /* Shared counter. */

void handler(void *ptr) {
  int x;
  x = *((int *) ptr);
  printf("Thread %d: Waiting to enter critical region...\n", x);
  sem_wait(&sem); /* Down semaphore */

  /* --------------------- */
  /* BEGIN CRITICAL REGION */
  /* --------------------- */
  printf("Thread %d: Now in critical region. Counter Value: %d\n", x, counter);
  printf("Thread %d: Incrementing Counter...\n", x);
  counter++;
  printf("Thread %d: New Counter Value: %d [Sleeping for %d seconds.]\n", x, counter, THREAD_SLEEP_SEC);
  sleep(THREAD_SLEEP_SEC);
  printf("Thread %d: Exiting critical region...\n\n", x);
  /* --------------------- */
  /*  END CRITICAL REGION  */
  /* --------------------- */

  sem_post(&sem); /* Up semaphore */

  pthread_exit(0);
}

int main()
{
  pthread_t threads[NTHREADS];
  int thread_tid[NTHREADS];

  /* Initialize the semaphore.
   * Param 1 means it will be shared amongst threads in the same process.
   * Param 2 initializes the semphore value. (A 1 would make a lock.)
   */
  sem_init(&sem, 0, SEMAPHORE_VAL);

  /* Run our threads, then join them. */
  int i;
  for (i = 0; i < NTHREADS; i++) {
    thread_tid[i] = i;
    if (pthread_create(&threads[i], NULL, (void *)handler, &thread_tid[i]) < 0)
      perror("error creating thread");
  }
  for (i = 0; i < NTHREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("Main Thread: Final Counter Value: %d\n", counter);

  /* Destroy the sem. */
  sem_destroy(&sem);

  exit(0);
}
