#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

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
 * allocated. If the block pointed to is removed by coalescing, this shouuld be
 * updated to point to the next block after the removed block.
 */
header *g_next_allocate = NULL;

/*
 * Direct the compiler to run the init function before running main
 * this allows initialization of required globals
 */

static void init(void) __attribute__((constructor));

/*
 * Direct the compiler to ignore unused static functions.
 * TODO: Remove these in your code.
 */
/*
static void set_fenceposts(void *mem, size_t size) __attribute__((unused));
static void insert_free_block(header *h) __attribute__((unused));
static header *find_header(size_t size) __attribute__((unused));
*/

/*
 * Func defn
 * here
 */

static size_t round_up_size(size_t size) {
  return ((size + MIN_ALLOCATION - 1) / MIN_ALLOCATION) * MIN_ALLOCATION;
} /* round_up_size() */

static void set_state(header *h, state s) {
  h->size = (h->size & ~0b111) | s;
}

static state get_state(header *h) {
  return (state)(h->size & 0b111);
}

static size_t get_size(header *h) {
  return h->size & ~0b111;
}

static header *get_header(void *p) {
    return (header *)((char *)p - ALLOC_HEADER_SIZE);
}

/*
 * TODO: implement first_fit
 * Allocate the first available block able to satisfy the request
 * (starting the search at g_freelist_head)
 */
/*
static header *first_fit(size_t size) {
  (void) size;
  assert(false);
  exit(1);
} *//* first_fit() */

/*
 *  TODO: implement next_fit
 *  Allocate the first available block able to satisfy the request
 *  (starting the search at the next free header after the header that was most
 *  recently allocated)
 */
/*
static header *next_fit(size_t size) {
  (void) size;
  assert(false);
  exit(1);
} *//* next_fit() */

/*
 * TODO: implement best_fit
 * Allocate the FIRST instance of the smallest block able to satisfy the
 * request
 */
/*
static header *best_fit(size_t size) {
  (void) size;
  assert(false);
  exit(1);
} *//* best_fit() */

/*
 * TODO: implement worst_fit
 */
/*
static header *worst_fit(size_t size) {
  (void) size;
  assert(false);
  exit(1);
} *//* worst_fit() */

/*
 * Returns the address of the block to allocate
 * based on the specified algorithm.
 *
 * If no block is available, returns NULL.
 */

/*
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
}  find_header()
*/

static header *find_header(size_t size) {
    for (header *h = g_freelist_head; h != NULL; h = h->next) {
        if (get_size(h) >= size) {
            return h;
        }
    }
    return NULL;
}

/*
 * Calculates the location of the left neighbor given a header.
 */

static inline header *left_neighbor(header *h) {
  return (header *) (((char *) h) - h->left_size - ALLOC_HEADER_SIZE);
} /* left_neighbor() */

/*
 * Calculates the location of the right neighbor given a header.
 */

static inline header *right_neighbor(header *h) {
  return (header *) (((char *) h) + ALLOC_HEADER_SIZE + TRUE_SIZE(h));
} /* right_neighbor() */

/*
 * Insert a block at the beginning of the freelist.
 * The block is located after its left header, h.
 */

static void insert_free_block(header *h) {
  h->prev = NULL;

  if (g_freelist_head != NULL) {
    g_freelist_head->prev = h;
  }

  h->next = g_freelist_head;
  g_freelist_head = h;
} /* insert_free_block() */

static void remove_from_freelist(header *h) {
  if (h == g_freelist_head) {
    g_freelist_head = h->next;
    if (g_freelist_head) {
      g_freelist_head->prev = NULL;
    }
  }
  else {
    if (h->prev) {
      h->prev->next = h->next;
    }
    if (h->next) {
      h->next->prev = h->prev;
    }
  }
}

/*
 * Instantiates fenceposts at the left and right side of a block.
 */

static void set_fenceposts(void *mem, size_t size) {
  header *left_fence = (header *) mem;
  header *right_fence = (header *) (((char *) mem) +
                         (size - ALLOC_HEADER_SIZE));

  left_fence->size = (state) FENCEPOST;
  right_fence->size = (state) FENCEPOST;

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

    header *far_right = right_neighbor(h);
    if (get_state(far_right) != FENCEPOST) {
        far_right->left_size = get_size(h);
    }

    return h;
}

void split_block(header *h, size_t alloc_size) {
    size_t total_size = get_size(h); // Total size including header
    size_t remaining_size = total_size - alloc_size - ALLOC_HEADER_SIZE;

    // Only split if the remaining size can form a valid block
    if (remaining_size > MIN_ALLOCATION) {
        header *new_block = (header *)((char *)h + alloc_size + ALLOC_HEADER_SIZE);
        new_block->size = remaining_size; // Exclude header size
        set_state(new_block, UNALLOCATED);
        new_block->left_size = alloc_size; // Size of allocated block excluding header
        insert_free_block(new_block);

        header *right_block = right_neighbor(new_block);
        if (right_block != NULL) {
            right_block->left_size = get_size(new_block);
        }

        h->size = alloc_size; // Exclude header size
    } 
    /*
    else {
        // Do not split; allocate the entire block
        h->size = total_size - ALLOC_HEADER_SIZE; // Adjust size in case of extra bytes
    }
    */
    set_state(h, ALLOCATED);
}

static void allocate_chunk(size_t size) {
    // Calculate the minimum chunk size needed, rounding up to ARENA_SIZE
    size_t chunk_size = ((size + 2 * ALLOC_HEADER_SIZE + ARENA_SIZE - 1) / ARENA_SIZE) * ARENA_SIZE;

    void *mem = sbrk(chunk_size);
    if (mem == (void *)-1) {
        errno = ENOMEM;
        return;
    }

    header *left_fence = (header *)mem;
    header *right_fence = (header *)((char *)mem + chunk_size - ALLOC_HEADER_SIZE);

    // Set up the fenceposts
    /*
    left_fence->size = FENCEPOST;
    set_state(left_fence, FENCEPOST);
    left_fence->left_size = 0;

    right_fence->size = FENCEPOST;
    set_state(right_fence, FENCEPOST);
    right_fence->left_size = h->size = chunk_size - 2 * ALLOC_HEADER_SIZE; // Exclude fenceposts
    */

    set_fenceposts(mem, chunk_size);
    
    header *h;
    // set_state(h, UNALLOCATED);
    // h->left_size = ALLOC_HEADER_SIZE; // Left neighbor is left fencepost

    // Coalesce with previous chunk if adjacent
    if (g_last_fence_post != NULL && (char *)left_fence == (char *)g_last_fence_post + ALLOC_HEADER_SIZE) {
        header * prev_block = (header *) ((char *)left_fence - g_last_fence_post->left_size - ALLOC_HEADER_SIZE);

	h = (header *) ((char *) left_fence - ALLOC_HEADER_SIZE);
	h->size = chunk_size - ALLOC_HEADER_SIZE;
	h->left_size = prev_block->size;

	// Get the previous block
        /*
	header *prev_block = (header *)((char *)left_fence - g_last_fence_post->left_size - ALLOC_HEADER_SIZE);

        // Remove previous block from free list if it's unallocated
        if (get_state(prev_block) == UNALLOCATED) {
	    remove_from_freelist(prev_block);

            // Extend the previous block to include the new chunk
            prev_block->size += h->size + 2 * ALLOC_HEADER_SIZE; // Include new chunk and fenceposts

            // Update the right fencepost's left_size
            right_fence->left_size = prev_block->size;

            // Insert the coalesced block back into the free list
            insert_free_block(prev_block);

            // Update g_last_fence_post
            g_last_fence_post = right_fence;

            // **Insert the OS chunk**
            // insert_os_chunk((header *)mem);

            return prev_block;
        } else {
            // Cannot coalesce, adjust left_size of the new chunk
            h->left_size = g_last_fence_post->size;
        }
	*/
    }
    else {
	h = (header *)((char *)mem + ALLOC_HEADER_SIZE);
    	h->size = chunk_size - 3 * ALLOC_HEADER_SIZE;
        h->left_size = ALLOC_HEADER_SIZE;
    }
    /*
    else {
        // Not adjacent, update g_last_fence_post
        g_last_fence_post = right_fence;
    }
    */
    
    set_state(h, UNALLOCATED);
    insert_free_block(h);

    // Insert the new chunk into the free list
    right_fence->left_size = h->size;
    g_last_fence_post = right_fence;

    // return h;
}


/*
 * TODO: implement malloc
 */

/*
void *my_malloc(size_t size) {
  pthread_mutex_lock(&g_mutex);
  // Insert code here
  pthread_mutex_unlock(&g_mutex);

  // Remove this code
  (void) size;
  assert(false);
  exit(1);
}  my_malloc()
*/

void *my_malloc(size_t size) {
    pthread_mutex_lock(&g_mutex);

    if (size == 0) {
        pthread_mutex_unlock(&g_mutex);
        return NULL;
    }

    // Use round_up_size to calculate the user size
    size_t alloc_size = round_up_size(size);

    // Total size including header
    // size_t alloc_size = user_size + ALLOC_HEADER_SIZE;

    header *h = find_header(alloc_size);

    if (h == NULL) {
        allocate_chunk(alloc_size);
        /*
	if (h == NULL) {
            pthread_mutex_unlock(&g_mutex);
            return NULL;
        }
	*/
        // No need to insert h into free list; allocate_chunk handles it
        h = find_header(alloc_size);
        if (h == NULL) {
            // Allocation failed
            pthread_mutex_unlock(&g_mutex);
            return NULL;
        }
    }

    split_block(h, alloc_size);
    remove_from_freelist(h);
    // set_state(h, ALLOCATED); // Already set in split_block

    pthread_mutex_unlock(&g_mutex);

    // Return pointer to user data (after header)
    return (void *)((char *)h + ALLOC_HEADER_SIZE);
}


/*
 * TODO: implement free
 */
/*
void my_free(void *p) {
  pthread_mutex_lock(&g_mutex);
  // Insert code here
  pthread_mutex_unlock(&g_mutex);

  // Remove this code
  (void) p;
  assert(false);
  exit(1);
}  my_free()
*/

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


/*
 * Calls malloc and sets each byte of
 * the allocated memory to a value.
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
