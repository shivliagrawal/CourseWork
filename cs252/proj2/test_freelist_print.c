#include <stdio.h>
#include "my_malloc.h" // Include your allocator implementation

// Define a print function for freelist_print to use
void print_object(header *h) {
    printf("Free block at %p: size=%zu, state=%s\n", (void *)h, get_size(h), 
           (get_state(h) == ALLOCATED ? "ALLOCATED" : "UNALLOCATED"));
}

int main() {
    printf("Starting memory allocator test...\n");

    // Allocate some memory blocks
    int *arr1 = (int *)my_malloc(100); // Allocate 100 bytes
    int *arr2 = (int *)my_malloc(200); // Allocate 200 bytes
    int *arr3 = (int *)my_malloc(300); // Allocate 300 bytes

    // Print the free list after allocations
    printf("\n--- Free list after allocations ---\n");
    freelist_print(print_object);

    // Free one of the blocks
    my_free(arr2);

    // Print the free list after freeing a block
    printf("\n--- Free list after freeing arr2 (200 bytes) ---\n");
    freelist_print(print_object);

    // Allocate another block
    int *arr4 = (int *)my_malloc(150); // Allocate 150 bytes

    // Print the free list after another allocation
    printf("\n--- Free list after allocating arr4 (150 bytes) ---\n");
    freelist_print(print_object);

    // Free all remaining blocks
    my_free(arr1);
    my_free(arr3);
    my_free(arr4);

    // Print the free list after freeing all blocks
    printf("\n--- Free list after freeing all blocks ---\n");
    freelist_print(print_object);

    printf("Memory allocator test completed.\n");
    return 0;
}
