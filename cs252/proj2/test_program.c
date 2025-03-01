#include <stdio.h>
#include "my_malloc.h"  // Include your malloc implementation

int main() {
    // Allocate some blocks to test the malloc and split behavior
    int *arr1 = (int *)my_malloc(64);  // Request 64 bytes
    int *arr2 = (int *)my_malloc(128); // Request 128 bytes
    
    // Print addresses of the allocated blocks to verify
    printf("arr1 allocated at: %p\n", (void *)arr1);
    printf("arr2 allocated at: %p\n", (void *)arr2);

    // Free the blocks
    my_free(arr1);
    my_free(arr2);

    return 0;
}

