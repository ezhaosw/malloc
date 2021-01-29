#include <stdio.h>
#include <assert.h>
#include "mm_alloc.h"

int main(int argc, char** argv) {
    int* data;
    data = (int*) mm_malloc(100);
    data[0] = 1;
    int* temp = data;
    mm_free(data);
    int* data1 = (int*) mm_malloc(30);
    int* data2 = (int*) mm_malloc(30);
    assert(temp == data1);
    printf("Basic test passed\n");
    return 0;
}
