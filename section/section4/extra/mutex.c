#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#define NTHREADS 10
#define THREAD_SLEEP_SEC 1

void *thread_function(void *);

/* Initialize a mutex, AKA a lock, AKA a binary semaphore. */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

int main() {
  pthread_t threads[NTHREADS];
  int thread_tid[NTHREADS];

  int i;
  for (i = 0; i < NTHREADS; i++) {
    thread_tid[i] = i;
    if (pthread_create(&threads[i], NULL, thread_function, &thread_tid[i]) < 0)
      perror("error creating thread");
  }
  for (i = 0; i < NTHREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  /* Now that all threads are complete we can print the final result. Without
   * the join, we could be printing the value before all the threads have been
   * completed.
   */
  printf("Final counter value: %d\n", counter);

  return 0;
}

void *thread_function(void *ptr) {
  int tid;
  tid = *((int *) ptr);
  printf("Thread %d: Attempting to grab mutex now...\n", tid);

  /* --------------------- */
  /* BEGIN CRITICAL REGION */
  /* --------------------- */
  pthread_mutex_lock(&mutex);
  printf("Thread %d: Acquired mutex. Counter value: %d\n", tid, counter);
  printf("Thread %d: Incrementing counter...\n", tid);
  counter++;
  printf("Thread %d: New Counter Value: %d [Sleeping for %d second.]\n\n", tid, counter, THREAD_SLEEP_SEC);
  sleep(THREAD_SLEEP_SEC);
  /* --------------------- */
  /*  END CRITICAL REGION  */
  /* --------------------- */

  pthread_mutex_unlock(&mutex);
  pthread_exit(0);
}
