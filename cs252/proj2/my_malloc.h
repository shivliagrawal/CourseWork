#ifndef MY_MALLOC_H
#define MY_MALLOC_H

#ifndef MIN_ALLOCATION
#define MIN_ALLOCATION 8
#endif

#ifndef ARENA_SIZE
#define ARENA_SIZE 4096
#endif

/*
 * Defines the algorithm that will be used to select the free block
 *
 * 1 = First Fit
 * 2 = Next Fit
 * 3 = Best Fit
 * 4 = Worst Fit
 */
#ifndef FIT_ALGORITHM
#define FIT_ALGORITHM 1
#endif

#define MIN(x, y) ((x < y) ? x : y)
#define MAX(x, y) ((x > y) ? x : y)

#define ALLOC_HEADER_SIZE (sizeof(header) - (2 * sizeof(header *)))

#define TRUE_SIZE(x) ((x->size) & ~0b111)

typedef enum state {
  UNALLOCATED = 0b000,
  ALLOCATED = 0b001,
  FENCEPOST = 0b010,
} state;

typedef struct header {
  size_t size;
  size_t left_size;
  union {
    struct {
      struct header *next;
      struct header *prev;
    };
    char data[0];
  };
} header;

void *my_malloc(size_t size);
void *my_calloc(size_t nmemb, size_t size);
void *my_realloc(void *ptr, size_t size);
void my_free(void *p);

extern void* g_base;
extern header* g_freelist_head;
extern header* g_next_allocate;

#endif
