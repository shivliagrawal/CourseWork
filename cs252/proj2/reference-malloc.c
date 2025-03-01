#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "my_malloc.h"

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

/*
 * Direct the compiler to run the init function before running main
 * this allows initialization of required globals
 */
static void init(void) __attribute__((constructor));

/*
 * Function declarations
 */
static void set_fenceposts(void *mem, size_t size);
static void insert_free_block(header *h);
static void remove_from_free_list(header *block);
static header *find_header(size_t size);
static header *allocate_chunk(size_t size);
static void split_block(header *block, size_t alloc_size);
static void coalesce_blocks(header *block);
static inline header *left_neighbor(header *h);
static inline header *right_neighbor(header *h);

/*
 * Allocation algorithms
 */
static header *first_fit(size_t size);
static header *next_fit(size_t size);
static header *best_fit(size_t size);
static header *worst_fit(size_t size);

/*
 * Calculates the location of the left neighbor given a header.
 */
static inline header *left_neighbor(header *h) {
  return (header *) (((char *) h) - h->left_size - ALLOC_HEADER_SIZE);
}

/*
 * Calculates the location of the right neighbor given a header.
 */
static inline header *right_neighbor(header *h) {
  return (header *) (((char *) h) + ALLOC_HEADER_SIZE + TRUE_SIZE(h));
}

/*
 * Insert a block at the beginning of the free list.
 */
static void insert_free_block(header *h) {
  h->prev = NULL;

  if (g_freelist_head != NULL) {
    g_freelist_head->prev = h;
  }

  h->next = g_freelist_head;
  g_freelist_head = h;
}

/*
 * Remove a block from the free list.
 */
static void remove_from_free_list(header *block) {
  if (block == g_freelist_head) {
    g_freelist_head = block->next;
    if (g_freelist_head) {
      g_freelist_head->prev = NULL;
    }
  } else {
    if (block->prev) {
      block->prev->next = block->next;
    }
    if (block->next) {
      block->next->prev = block->prev;
    }
  }
}

/*
 * Instantiates fenceposts at the left and right side of a block.
 */
static void set_fenceposts(void *mem, size_t size) {
  header *left_fence = (header *) mem;
  header *right_fence = (header *) (((char *) mem) + (size - ALLOC_HEADER_SIZE));

  left_fence->size = (state) FENCEPOST;
  right_fence->size = (state) FENCEPOST;

  right_fence->left_size = size - 3 * ALLOC_HEADER_SIZE;
}

/*
 * Constructor that runs before main() to initialize the library.
 */
static void init() {
  g_freelist_head = NULL;

  /* Initialize mutex for thread safety */
  pthread_mutex_init(&g_mutex, NULL);

  /* Manually set printf buffer so it won't call malloc */
  setvbuf(stdout, NULL, _IONBF, 0);

  /* Record the starting address of the heap */
  g_base = sbrk(0);
}

static header *allocate_chunk(size_t size) {

  size_t overhead = 3 * ALLOC_HEADER_SIZE; // For fenceposts and block header
  size_t required_size = size + overhead;
  size_t chunk_size = ((required_size + ARENA_SIZE - 1) / ARENA_SIZE) * ARENA_SIZE;
  printf("Alloc header size: %zu\n", ALLOC_HEADER_SIZE);
  printf("Chunk size requested: %zu\n", chunk_size);


  void *mem = sbrk(chunk_size);
  if (mem == (void *) -1) {
    return NULL; // Failed to allocate memory from OS
  }

  set_fenceposts(mem, chunk_size);

  header *h;
  header * left_fence = (header *)mem;
  header * right_fence = (header *)((char *)mem + chunk_size - ALLOC_HEADER_SIZE);

    // Coalesce with previous chunk if adjacent
    if (g_last_fence_post != NULL && (char *)left_fence == (char *)g_last_fence_post + ALLOC_HEADER_SIZE) {
        // Get the previous block
        header *prev_block = (header *)((char *)g_last_fence_post - g_last_fence_post->left_size - ALLOC_HEADER_SIZE);

        // Remove previous block from free list if it's unallocated
        if (!(prev_block->size & ALLOCATED)) {
            // Extend the previous block to include the new chunk
            prev_block->size += chunk_size;  // Include the size of the new chunk (fenceposts already accounted for)

            // Set the right fencepost's left_size
            right_fence->left_size = prev_block->size;

            // Update g_last_fence_post to point to the new right fencepost
            g_last_fence_post = right_fence;

            return prev_block;  // Return the coalesced block
        } else {
            // Cannot coalesce, adjust left_size of the new chunk
            h = (header *)((char *)left_fence - ALLOC_HEADER_SIZE);
            h->left_size = prev_block->size;
        }
    } else {
        // Not adjacent, create a new free block
        h = (header *)((char *)mem + ALLOC_HEADER_SIZE);
        h->size = chunk_size - 3 * ALLOC_HEADER_SIZE; // Subtract the size of both fenceposts and block header
        h->left_size = 0; // Left neighbor is the left fencepost
    }
    h->size &= ~0b111;
    // Insert the new block into the free list
    insert_free_block(h);

    // Update the right fencepost to reflect the new chunk
    g_last_fence_post = right_fence;
    right_fence->left_size = h->size;

    return h;
}

/*
 * Split a block if possible.
 */


static void split_block(header *block, size_t alloc_size) {
  size_t total_size = TRUE_SIZE(block);
  size_t remaining_size = total_size - alloc_size - ALLOC_HEADER_SIZE;

  if (remaining_size >= 2 * sizeof(header *)) {  
  /* Split block */
    header *remaining_block = (header *) ((char *) block + ALLOC_HEADER_SIZE + alloc_size);
    remaining_block->size = remaining_size;
    remaining_block->left_size = alloc_size;

    /* Clear state bits */
    remaining_block->size &= ~0b111;

    /* Insert the remaining block into the free list */
    insert_free_block(remaining_block);

    /* Update the right neighbor's left_size */
    header *right_block = right_neighbor(remaining_block);
    right_block->left_size = remaining_block->size;
  } else {
    /* Do not split; allocate the entire block */
    alloc_size = total_size; // Adjust alloc_size to the full block size
    /* Remove block from free list */
    // Block is already removed in my_malloc
    /* Update the right neighbor's left_size */
    header *right_block = right_neighbor(block);
    right_block->left_size = block->size;
  }

  /* Set block's size */
  block->size = alloc_size;
}



/*
 * Coalesce a block with its neighbors if possible.
 */
/*
static void coalesce_blocks(header *block) {
  // Coalesce with right neighbor if it's free and not a fencepost
  header *right_block = right_neighbor(block);
  if (!(right_block->size & FENCEPOST) && !(right_block->size & ALLOCATED)) {
    // Remove right block from the free list
    remove_from_free_list(right_block);

    // Extend current block
    block->size += TRUE_SIZE(right_block) + ALLOC_HEADER_SIZE;
  }

  // Coalesce with left neighbor if it's free and not a fencepost
  header *left_block = left_neighbor(block);
  if (!(left_block->size & FENCEPOST) && !(left_block->size & ALLOCATED)) {
    // Remove left block from the free list
    remove_from_free_list(left_block);

    // Extend left block
    left_block->size += TRUE_SIZE(block) + ALLOC_HEADER_SIZE;
    block = left_block;
  }

  // Update the right neighbor's left_size
  header *next_block = right_neighbor(block);
  next_block->left_size = block->size;

  // Insert the (possibly coalesced) block into the free list
  insert_free_block(block);
}
*/


static void coalesce_blocks(header *block) {
  int coalesced_right = 0; // Indicator for coalescing with the right neighbor
  int coalesced_left = 0;
  header *left_block = left_neighbor(block);
  header *right_block = right_neighbor(block);

  // Coalesce with right neighbor if it's free and not a fencepost
  if (!(right_block->size & FENCEPOST) && !(right_block->size & ALLOCATED)) {
    if (g_next_allocate == right_block) {
      g_next_allocate = right_block->next ? right_block->next : g_freelist_head;
    }
    // Remove the right block from the free list and coalesce
    block->size += TRUE_SIZE(right_block) + ALLOC_HEADER_SIZE;

    // The current block should take the place of the right block in the free list
    block->next = right_block->next;
    block->prev = right_block->prev;
    if (block->prev) {
      block->prev->next = block;
    }
    if (block->next) {
      block->next->prev = block;
    }

    coalesced_right = 1; // Indicate that we coalesced with the right neighbor
  }

  // Coalesce with left neighbor if it's free and not a fencepost
  if (!(left_block->size & FENCEPOST) && !(left_block->size & ALLOCATED)) {
    if (g_next_allocate == left_block) {
      g_next_allocate = left_block->next ? left_block->next : g_freelist_head;
    }
    // Coalesce with the left block without changing its position in the free list
    left_block->size += TRUE_SIZE(block) + ALLOC_HEADER_SIZE;
    block = left_block; // The coalesced block starts at left_block

    // No need to manipulate the free list; left_block remains in the same position
    coalesced_left = 1;
  }

  // Update the right neighbor's left_size for the coalesced block
  header *next_block = right_neighbor(block);
  next_block->left_size = block->size;

  // Insert into the free list only if no coalescing happened with the right neighbor
  if (!coalesced_right && !coalesced_left) {
    insert_free_block(block);
  }
}



/*
 * Allocation algorithms implementations
 */
static header *first_fit(size_t size) {
  header *current = g_freelist_head;
  while (current) {
    if (current->size >= size) {
      return current; // Found suitable block
    }
    current = current->next;
  }
  return NULL; // No suitable block found
}

static header *next_fit(size_t size) {
  if (!g_freelist_head) {
    return NULL; // No blocks in the free list
  }

  if (!g_next_allocate) {
    g_next_allocate = g_freelist_head; // Start from the head if no previous allocation
  }

  header *current = g_next_allocate;
  header *starting_point = g_next_allocate; // Remember the starting point for circular traversal

  /* Loop through free list circularly */
  do {
    if (TRUE_SIZE(current) >= size) {
      /* Found a suitable block */
      g_next_allocate = current->next ? current->next : g_freelist_head; // Move the pointer for next allocation
      return current;
    }
    current = current->next ? current->next : g_freelist_head; // Move to the next block, wrap around if necessary
  } while (current != starting_point); // Stop when we have completed one full cycle

  return NULL; // No suitable block found
}

static header *worst_fit(size_t size) {
  header *current = g_freelist_head;
  header *worst_block = NULL;

  while (current) {
    if (current->size >= size && (!worst_block || current->size > worst_block->size)) {
      worst_block = current;
    }
    current = current->next;
  }
  return worst_block;
}

static header *best_fit(size_t size) {
  header *current = g_freelist_head;
  header *best_block = NULL;

  while (current) {
    if (current->size >= size && (!best_block || current->size < best_block->size)) {
      best_block = current;
    }
    current = current->next;
  }
  return best_block;
}


/*
 * Returns the address of the block to allocate
 * based on the specified algorithm.
 *
 * If no block is available, returns NULL.
 */
static header *find_header(size_t size) {
  if (g_freelist_head == NULL) {
    return NULL;
  }

  switch (FIT_ALGORITHM) {
    case 1:
      return first_fit(size);
    case 2:
      return next_fit(size);
    case 3:
      return best_fit(size);
    case 4:
      return worst_fit(size);
  }
  assert(false);
  return NULL;
}

/*
 * Implement malloc
 */
void *my_malloc(size_t size) {
  pthread_mutex_lock(&g_mutex); // Lock mutex

  if (size == 0) { // Return NULL if size 0
    pthread_mutex_unlock(&g_mutex);
    return NULL;
  }

  size_t alloc_size = ((size + MIN_ALLOCATION - 1) / MIN_ALLOCATION) * MIN_ALLOCATION;
  printf("Requested allocation size: %zu, adjusted to %zu\n", size, alloc_size);
  printf("Arena Size: %u\n", ARENA_SIZE);

  header *block = find_header(alloc_size);
  if (block) {
    printf("Found a suitable block at address: %p, size: %zu\n", block, TRUE_SIZE(block));
  } else {
    printf("No suitable block found in free list, requesting more memory from OS.\n");
    block = allocate_chunk(alloc_size);
    if (!block) {
      pthread_mutex_unlock(&g_mutex);
      errno = ENOMEM;
      return NULL; // Failed to allocate memory from OS
    }
  }

  remove_from_free_list(block);

  /* Allocate block (fully/split) */
  split_block(block, alloc_size);

  /* Mark block as allocated */
  block->size = block->size | ALLOCATED;
  pthread_mutex_unlock(&g_mutex);

  /* Return data pointer after the header */
  printf("Returning pointer to data at address: %p\n", (void *) block->data);
  return (void *) block->data;
}

/*
 * Implement free
 */


void my_free(void *p) {
  if (!p) {
    printf("Attempted to free a NULL pointer, returning.\n");
    return; // Freeing a NULL pointer is a no-op
  }

  pthread_mutex_lock(&g_mutex);

  /* Retrieve header by subtracting size of header from p */
  header *block = (header *) ((char *) p - ALLOC_HEADER_SIZE);
  printf("Freeing block at address: %p, size: %zu\n", block, TRUE_SIZE(block));

  /* Check if the block is already freed */
  if (!(block->size & ALLOCATED)) {
    printf("Error: Attempted to free an already freed block at address: %p\n", block);
    pthread_mutex_unlock(&g_mutex);
    exit(EXIT_FAILURE); // Stop further execution due to the error
  }

  /* Mark the block as free (clear the allocated bit) */
  block->size &= ~ALLOCATED;
  printf("Block at address: %p marked as free.\n", block);

  /* Coalesce with neighbors if possible */
  coalesce_blocks(block);

  pthread_mutex_unlock(&g_mutex);
}



/*
 * Calls malloc and sets each byte of
 * the allocated memory to a value.
 */
void *my_calloc(size_t nmemb, size_t size) {
  return memset(my_malloc(size * nmemb), 0, size * nmemb);
}

/*
 * Reallocates an allocated block to a new size and
 * copies the contents to the new block.
 */
void *my_realloc(void *ptr, size_t size) {
  void *mem = my_malloc(size);
  memcpy(mem, ptr, size);
  my_free(ptr);
  return mem;
}

