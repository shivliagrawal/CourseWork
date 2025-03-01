#include <stdio.h>
#include "my_malloc.h"
#include "printing.h"

int main() {
    // Allocate a block larger than needed and ensure it's split
    int *arr = (int *)my_malloc(64 * sizeof(int)); // Request 64 * sizeof(int)
    
    freelist_print(print_object); // Print free list before allocation

    int *arr2 = (int *)my_malloc(16 * sizeof(int)); // Request smaller block to force split

    freelist_print(print_object); // Print free list after allocation to check for the split

    return 0;
}

