// test_allocator.c
#include <stdio.h>
#include <assert.h>
#include "my_malloc.h"
#include "printing.h"

int main() {
    printf("Testing my_malloc...\n");

    // Example test cases
    int *arr = (int *)my_malloc(100 * sizeof(int));
    assert(arr != NULL);

    // Print the free list after allocation
    freelist_print(print_object);

    // Free the allocated memory
    my_free(arr);

    // Print the free list after freeing
    freelist_print(print_object);

    return 0;
}

