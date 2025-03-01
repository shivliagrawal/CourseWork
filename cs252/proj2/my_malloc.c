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
 * the OS. Used for coalescing chunks.
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
 * this allows initialization of required globals.
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
  return (header *)(((char *)h) - h->left_size - ALLOC_HEADER_SIZE);
} /* left_neighbor() */

/*
 * Calculates the location of the right neighbor given a header.
 */
static inline header *right_neighbor(header *h) {
  return (header *)(((char *)h) + ALLOC_HEADER_SIZE + TRUE_SIZE(h));
} /* right_neighbor() */

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
} /* insert_free_block() */

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
} /* remove_from_free_list() */

/*
 * Instantiates fenceposts at the left and right side of a block.
 */
static void set_fenceposts(void *mem, size_t size) {
  header *left_fence = (header *)mem;
  header *right_fence = (header *)(((char *)mem) + (size - ALLOC_HEADER_SIZE));

  left_fence->size = (state)FENCEPOST;
  right_fence->size = (state)FENCEPOST;

  right_fence->left_size = size - 3 * ALLOC_HEADER_SIZE;
} /* set_fenceposts() */

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
} /* init() */

/*
 * Allocate a new chunk when no block is available.
 */
static header *allocate_chunk(size_t size) {
  size_t overhead = 3 * ALLOC_HEADER_SIZE;
  size_t required_size = size + overhead;
  size_t chunk_size = ((required_size + ARENA_SIZE - 1) / ARENA_SIZE) * ARENA_SIZE;

  void *mem = sbrk(chunk_size);
  if (mem == (void *)-1) {
    return NULL; /* Failed to allocate memory from OS */
  }

  set_fenceposts(mem, chunk_size);

  header *h;
  header *left_fence = (header *)mem;
  header *right_fence = (header *)((char *)mem + chunk_size - ALLOC_HEADER_SIZE);

  if (g_last_fence_post != NULL && (char *)left_fence == (char *)g_last_fence_post + ALLOC_HEADER_SIZE) {
    header *prev_block = (header *)((char *)g_last_fence_post - g_last_fence_post->left_size - ALLOC_HEADER_SIZE);

    if (!(prev_block->size & ALLOCATED)) {
      prev_block->size += chunk_size;
      right_fence->left_size = prev_block->size;
      g_last_fence_post = right_fence;
      return prev_block;
    } else {
      h = (header *)((char *)left_fence - ALLOC_HEADER_SIZE);
      h->left_size = prev_block->size;
    }
  } else {
    h = (header *)((char *)mem + ALLOC_HEADER_SIZE);
    h->size = chunk_size - 3 * ALLOC_HEADER_SIZE;
    h->left_size = 0;
  }

  insert_free_block(h);
  g_last_fence_post = right_fence;
  right_fence->left_size = h->size;

  return h;
} /* allocate_chunk() */

/*
 * Implement split block.
 */
static void split_block(header *block, size_t alloc_size) {
  size_t total_size = TRUE_SIZE(block);
  size_t remaining_size = total_size - alloc_size - ALLOC_HEADER_SIZE;

  if (remaining_size >= 2 * sizeof(header *)) {
    header *remaining_block = (header *)((char *)block + ALLOC_HEADER_SIZE + alloc_size);
    remaining_block->size = remaining_size;
    remaining_block->left_size = alloc_size;

    remaining_block->size &= ~0b111;
    insert_free_block(remaining_block);

    header *right_block = right_neighbor(remaining_block);
    right_block->left_size = remaining_block->size;
  } else {
    alloc_size = total_size;
    header *right_block = right_neighbor(block);
    right_block->left_size = block->size;
  }

  block->size = alloc_size;
} /* split_block() */

/*
 * Coalesce a block with its neighbors if possible.
 */
static void coalesce_blocks(header *block) {
  header *right_block = right_neighbor(block);
  if (!(right_block->size & FENCEPOST) && !(right_block->size & ALLOCATED)) {
    remove_from_free_list(right_block);
    block->size += TRUE_SIZE(right_block) + ALLOC_HEADER_SIZE;
  }

  header *left_block = left_neighbor(block);
  if (!(left_block->size & FENCEPOST) && !(left_block->size & ALLOCATED)) {
    remove_from_free_list(left_block);
    left_block->size += TRUE_SIZE(block) + ALLOC_HEADER_SIZE;
    block = left_block;
  }

  header *next_block = right_neighbor(block);
  next_block->left_size = block->size;
  insert_free_block(block);
} /* coalesce_blocks() */

/*
 * First fit implementation.
 */
static header *first_fit(size_t size) {
  header *current = g_freelist_head;
  while (current) {
    if (current->size >= size) {
      return current;
    }
    current = current->next;
  }
  return NULL;
} /* first_fit() */

/*
 * Next fit implementation.
 */
static header *next_fit(size_t size) {
  if (!g_freelist_head) {
    return NULL;
  }

  if (!g_next_allocate) {
    g_next_allocate = g_freelist_head;
  }

  header *current = g_next_allocate;
  header *starting_point = g_next_allocate;

  do {
    if (current->size >= size) {
      g_next_allocate = current->next ? current->next : g_freelist_head;
      return current;
    }
    current = current->next ? current->next : g_freelist_head;
  } while (current != starting_point);

  return NULL;
} /* next_fit() */

/*
 * Best fit implementation.
 */
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
} /* best_fit() */

/*
 * Worst fit implementation.
 */
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
} /* worst_fit() */

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
} /* find_header() */

/*
 * Implement malloc.
 */
void *my_malloc(size_t size) {
  pthread_mutex_lock(&g_mutex);

  if (size == 0) {
    pthread_mutex_unlock(&g_mutex);
    return NULL;
  }

  size_t alloc_size;
  if (size <= MIN_ALLOCATION) {
    alloc_size = 2 * sizeof(header *);
    alloc_size = ((alloc_size + MIN_ALLOCATION - 1) / MIN_ALLOCATION) * MIN_ALLOCATION;
  } else {
    alloc_size = ((size + MIN_ALLOCATION - 1) / MIN_ALLOCATION) * MIN_ALLOCATION;
  }

  header *block = find_header(alloc_size);
  if (!block) {
    block = allocate_chunk(alloc_size);
    if (!block) {
      pthread_mutex_unlock(&g_mutex);
      errno = ENOMEM;
      return NULL;
    }
  }

  remove_from_free_list(block);
  split_block(block, alloc_size);
  block->size = block->size | ALLOCATED;

  pthread_mutex_unlock(&g_mutex);
  return (void *)block->data;
} /* my_malloc() */

/*
 * Implement free.
 */
void my_free(void *p) {
  if (!p) {
    return;
  }

  pthread_mutex_lock(&g_mutex);

  header *block = (header *)((char *)p - ALLOC_HEADER_SIZE);

  if (!(block->size & ALLOCATED)) {
    pthread_mutex_unlock(&g_mutex);
    exit(EXIT_FAILURE);
  }

  block->size &= ~ALLOCATED;
  coalesce_blocks(block);

  pthread_mutex_unlock(&g_mutex);
} /* my_free() */

/*
 * Calls malloc and sets each byte of the allocated memory to a value.
 */
void *my_calloc(size_t nmemb, size_t size) {
  return memset(my_malloc(size * nmemb), 0, size * nmemb);
} /* my_calloc() */

/*
 * Reallocates an allocated block to a new size and
 * copies the contents to the new block.
 */
void *my_realloc(void *ptr, size_t size) {
  void *mem = my_malloc(size);
  memcpy(mem, ptr, size);
  my_free(ptr);
  return mem;
} /* my_realloc() */

