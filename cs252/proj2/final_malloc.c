#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "my_malloc.h"

#define MAX_OS_CHUNKS 1024

/* Pointer to the location of the heap prior to any sbrk calls */
void *g_base = NULL;

/* Pointer to the head of the free list */
header *g_freelist_head = NULL;

/* Mutex to ensure thread safety for the freelist */
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Pointer to the second fencepost in the most recently allocated chunk from
 * the OS. Used for coalescing chunks
 */
header *g_last_fence_post = NULL;

/*
 * Pointer to the next block in the freelist after the block that was last
 * allocated. If the block pointed to is removed by coalescing, this should be
 * updated to point to the next block after the removed block.
 */
header *g_next_allocate = NULL;

/* Array to keep track of OS-allocated chunks */
static header *os_chunk_list[MAX_OS_CHUNKS];
static size_t num_os_chunks = 0;

// Function declarations
static inline header *left_neighbor(header *h);
static inline header *right_neighbor(header *h);
static header *find_header(size_t size);

/* Helper function to round up to the nearest multiple of MIN_ALLOCATION */
static size_t round_up_size(size_t size) {
    return ((size + MIN_ALLOCATION - 1) / MIN_ALLOCATION) * MIN_ALLOCATION;
}

/* Helper function to get the header from a pointer */
static header *get_header(void *p) {
    return (header *)((char *)p - ALLOC_HEADER_SIZE);
}

/* Helper function to set the allocation state of a header */
static void set_state(header *h, state s) {
    h->size = (h->size & ~0x3) | s;
}

/* Helper function to get the allocation state of a header */
static state get_state(header *h) {
    return (state)(h->size & 0x3);
}

/* Helper function to get the true size of a block */
static size_t get_size(header *h) {
    return h->size & ~0x3;
}

static inline header *left_neighbor(header *h) {
    return (header *)((char *)h - h->left_size - ALLOC_HEADER_SIZE);
}

static inline header *right_neighbor(header *h) {
    return (header *)((char *)h + get_size(h) + ALLOC_HEADER_SIZE);
}

/* Helper function to remove a block from the free list */
static void remove_from_freelist(header *h) {
    if (h->prev) {
        h->prev->next = h->next;
    } else {
        g_freelist_head = h->next;
    }
    if (h->next) {
        h->next->prev = h->prev;
    }
}

/* Helper function to insert a block into the free list */
static void insert_free_block(header *h) {
    h->prev = NULL;
    h->next = g_freelist_head;
    if (g_freelist_head) {
        g_freelist_head->prev = h;
    }
    g_freelist_head = h;
}

/* Helper function to coalesce free blocks */
static header *coalesce(header *h) {
    header *left = left_neighbor(h);
    header *right = right_neighbor(h);

    if (get_state(left) == UNALLOCATED) {
        remove_from_freelist(left);
        left->size += get_size(h) + ALLOC_HEADER_SIZE;
        h = left;
    }

    if (get_state(right) == UNALLOCATED) {
        remove_from_freelist(right);
        h->size += get_size(right) + ALLOC_HEADER_SIZE;
    }

    right_neighbor(h)->left_size = get_size(h);
    return h;
}

static header *find_header(size_t size) {
    for (header *h = g_freelist_head; h != NULL; h = h->next) {
        if (get_size(h) >= size) {
            return h;
        }
    }
    return NULL;
}

/* Helper function to split a block if necessary */
static void split_block(header *h, size_t size) {
    size_t remaining_size = get_size(h) - size;
    if (remaining_size >= MIN_ALLOCATION + ALLOC_HEADER_SIZE) {
        header *new_block = (header *)((char *)h + size);
        new_block->size = remaining_size - ALLOC_HEADER_SIZE;
        set_state(new_block, UNALLOCATED);
        new_block->left_size = size;
        insert_free_block(new_block);

        header *right_block = right_neighbor(new_block);
        right_block->left_size = get_size(new_block);

        h->size = size;
    }
    set_state(h, ALLOCATED);
}

static void insert_os_chunk(header *h) {
    if (num_os_chunks < MAX_OS_CHUNKS) {
        os_chunk_list[num_os_chunks++] = h;
    }
}

/* Helper function to allocate a new chunk from the OS */
static header *allocate_chunk(size_t size) {
    size_t chunk_size = (size > ARENA_SIZE) ? size : ARENA_SIZE;
    void *mem = sbrk(chunk_size);
    if (mem == (void *)-1) {
        errno = ENOMEM;  // Set errno to ENOMEM when sbrk fails
        return NULL;
    }

    // Create left fencepost
    header *left_fence = (header *)mem;
    left_fence->size = FENCEPOST;
    left_fence->left_size = 0;

    // Create the free block
    header *h = (header *)((char *)mem + ALLOC_HEADER_SIZE);
    h->size = chunk_size - 2 * ALLOC_HEADER_SIZE; // Account for fenceposts
    set_state(h, UNALLOCATED);
    h->left_size = ALLOC_HEADER_SIZE; // Fencepost size

    // Create right fencepost
    header *right_fence = (header *)((char *)mem + chunk_size - ALLOC_HEADER_SIZE);
    right_fence->size = FENCEPOST;
    right_fence->left_size = get_size(h); // Set the left_size to the free block's size
    g_last_fence_post = right_fence; // Update the global last fencepost pointer

    // Insert the new chunk into the OS chunk list for tracking
    insert_os_chunk(left_fence);

    return h;
}

/* my_malloc function */
void *my_malloc(size_t size) {
    pthread_mutex_lock(&g_mutex);
    
    if (size == 0) {
        pthread_mutex_unlock(&g_mutex);
        return NULL;
    }

    size_t alloc_size = round_up_size(size + ALLOC_HEADER_SIZE);
    header *h = find_header(alloc_size);

    // If no block found, request a new chunk from the OS
    if (h == NULL) {
        h = allocate_chunk(alloc_size);
        if (h == NULL) {
            // If allocate_chunk failed (sbrk() failed), return NULL and set errno to ENOMEM
            errno = ENOMEM;
            pthread_mutex_unlock(&g_mutex);
            return NULL;
        }
        insert_free_block(h);  // Insert the newly allocated chunk into the free list
    }

    // Remove the block from the free list
    remove_from_freelist(h);

    // If the block size is larger than needed, split it
    split_block(h, alloc_size);
    
    // Mark the block as allocated
    set_state(h, ALLOCATED);

    pthread_mutex_unlock(&g_mutex);

    return (void *)((char *)h + ALLOC_HEADER_SIZE);
}


/* my_free function */
void my_free(void *p) {
    if (p == NULL) {
        return;
    }

    pthread_mutex_lock(&g_mutex);

    header *h = get_header(p);
    set_state(h, UNALLOCATED);

    h = coalesce(h);
    insert_free_block(h);

    pthread_mutex_unlock(&g_mutex);
}

/* my_calloc function */
void *my_calloc(size_t nmemb, size_t size) {
    return memset(my_malloc(size * nmemb), 0, size * nmemb);
}

/* my_realloc function */
void *my_realloc(void *ptr, size_t size) {
    void *mem = my_malloc(size);
    if (ptr != NULL && mem != NULL) {
        memcpy(mem, ptr, size);
        my_free(ptr);
    }
    return mem;
}

/* Initialization function */
static void init(void) __attribute__((constructor));
static void init(void) {
    g_freelist_head = NULL;
    pthread_mutex_init(&g_mutex, NULL);
    setvbuf(stdout, NULL, _IONBF, 0);
    g_base = sbrk(0);
}

