/**
 * A simple test harness for memory alloction. You should augment this with your
 * own tests.
 */

#include <stdio.h>
#include "mm_alloc.h"

int main(int argc, char **argv) {
    int *data;
    data = (int *) mm_malloc(4);
    data[0] = 1;
    mm_free(data);
    printf("malloc() basic test passed!\n");
    return 0;
}
