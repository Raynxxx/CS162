#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void *helper(void* arg) {
    printf("Hello World! 1\n");
    pthread_exit(0);
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, &helper, NULL);
    /* pthread_join(thread, NULL); */
    printf("Hello World! 2\n");
    exit(0);
}
