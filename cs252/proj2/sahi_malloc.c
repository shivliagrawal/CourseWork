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

/*
 * Allocate a new chunk from the OS.
 */
static header *allocate_chunk(size_t size) {
  size_t chunk_size;
  if (size >= ARENA_SIZE - 3 * ALLOC_HEADER_SIZE) {
    chunk_size = 2 * ARENA_SIZE;
  } else {
    chunk_size = ARENA_SIZE;
  }

  void *mem = sbrk(chunk_size);
  if (mem == (void *) -1) {
    return NULL; // Failed to allocate memory from OS
  }

  /* Set up fenceposts */
  set_fenceposts(mem, chunk_size);

  header *block = (header *) ((char *) mem + ALLOC_HEADER_SIZE); // Skip the left fencepost
  block->size = chunk_size - 3 * ALLOC_HEADER_SIZE; // Subtract left and right fenceposts
  block->left_size = 0; // No left neighbor initially

  /* Insert the new block into the free list */
  insert_free_block(block);

  /* Set the right fencepost as the last fencepost */
  g_last_fence_post = (header *) ((char *) block + block->size + ALLOC_HEADER_SIZE);

  return block;
}

/*
 * Split a block if possible.
 */
static void split_block(header *block, size_t alloc_size) {
  size_t true_block_size = TRUE_SIZE(block);
  size_t remaining_size = true_block_size - alloc_size - ALLOC_HEADER_SIZE;

  if (remaining_size > MIN_ALLOCATION) {
    /* Split block */
    header *remaining_block = (header *) ((char *) block + ALLOC_HEADER_SIZE + alloc_size);
    remaining_block->size = remaining_size;
    remaining_block->left_size = alloc_size;

    block->size = alloc_size;

    /* Update free list and neighbors */
    insert_free_block(remaining_block);
    g_last_fence_post->left_size = remaining_block->size;
  } else {
    /* Remove block from free list as it will be fully allocated */
    remove_from_free_list(block);
  }
}

/*
 * Coalesce a block with its neighbors if possible.
 */
static void coalesce_blocks(header *block) {
  int coalesced = 0;

  /* Coalesce with right neighbor if it's free and not a fencepost */
  header *right_block = right_neighbor(block);
  if (!(right_block->size & FENCEPOST) && !(right_block->size & ALLOCATED)) {
    /* Remove right block from the free list */
    remove_from_free_list(right_block);

    /* Extend current block */
    block->size += TRUE_SIZE(right_block) + ALLOC_HEADER_SIZE;
    coalesced = 1;
  }

  /* Coalesce with left neighbor if it's free and not a fencepost */
  header *left_block = left_neighbor(block);
  if (!(left_block->size & FENCEPOST) && !(left_block->size & ALLOCATED)) {
    /* Remove left block from the free list */
    remove_from_free_list(left_block);

    /* Extend left block */
    left_block->size += TRUE_SIZE(block) + ALLOC_HEADER_SIZE;
    block = left_block;
    coalesced = 1;
  }

  /* Insert the (possibly coalesced) block into the free list */
  insert_free_block(block);
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
    if (current->size >= size) {
      /* Found a suitable block */
      g_next_allocate = current->next ? current->next : g_freelist_head; // Move the pointer for next allocation
      return current;
    }
    current = current->next ? current->next : g_freelist_head; // Move to the next block, wrap around if necessary
  } while (current != starting_point); // Stop when we have completed one full cycle

  return NULL; // No suitable block found
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

  /* Mark the block as free (clear the allocated bit) */
  block->size = TRUE_SIZE(block) & ~ALLOCATED;
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

