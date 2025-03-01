#include "part1.h"

#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// The producers add the characters in g_prod_str to the g_buffer, in order.

char *g_prod_str = "The greatest teacher, failure is.";

bounded_buffer g_buffer;

// This mutex must be held whenever you use the g_buffer.

pthread_mutex_t g_buffer_mutex;


// g_empty_sem is the number of characters in the g_buffer,
// that need to be emptied.
// Producers should signal g_empty_sem and consumers should wait on it.

sem_t g_empty_sem;

// g_full_sem is the opposite of g_empty_sem. It is the number of
// unfilled slots in the g_buffer that need to be filled. Consumers
// should signal it, and producers should wait on it.

sem_t g_full_sem;

/*
 * Produce the characters (that is, add them to the g_buffer) from g_prod_str,
 *  in order. Signal consumers appropriately after each character.
 * Receive an ID via an (int *).
 */

void *producer(void *ptr) {
  int thread_id = *((int *) ptr);
  free(ptr);
  ptr = NULL;

  printf("Producer %d starting\n", thread_id);
  fflush(NULL);

  for (size_t i = 0; i < strlen(g_prod_str); i++) {
    // Add your code to wait on the semaphore and obtain the lock,
    // then add g_prod_str[i] to the g_buffer.

    printf("Thread %d produced %c\n", thread_id, g_prod_str[i]);
  }

  pthread_exit(0);
} /* producer() */

/*
 * Consume characters from the g_buffer. Stop after consuming the length of
 * g_prod_str, meaning that if an equal number of consumers and producers are
 * started, they will all exit. Signal producers appropriately of new free
 * space in the g_buffer. Receive and ID as an argument via an (int *).
 */

void *consumer(void *ptr) {
  int thread_id = *((int *) ptr);
  free(ptr);
  ptr = NULL;

  printf("Consumer %d starting\n", thread_id);
  fflush(NULL);

  for (size_t i = 0; i < strlen(g_prod_str); i++) {
    // Add your code to wait on the semaphore and obtain the lock,
    // then consume g_prod_str[i] from the g_buffer, replacing
    // the following line.

    char c = g_prod_str[i];

    printf("Thread %d consumed %c\n", thread_id, c);
  }

  pthread_exit(0);
} /* consumer() */

/*
 * Start a number of producers indicated by the first argument, and a number
 * of consumers indicated by the second argument.
 * Wait on all threads at the end to prevent premature exit.
 */

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Please pass two arguments.\n");
    exit(1);
  }

  // Remove the two following stataments when implementing your code

  (void)(argc);
  (void)(argv);

  // Initialize the g_buffer_mutex and condition variables

  pthread_mutex_init(&g_buffer_mutex, NULL);

  // Since the g_buffer starts out with no characters,
  // g_empty_sem should be 0 and g_full_sem should be the full
  // size of the g_buffer.

  sem_init(&g_empty_sem, 0, 0);
  sem_init(&g_full_sem, 0, BUF_SIZE);

  // Add your code to create the threads.
  // Make sure to allocate and pass the arguments correctly.


  // Add your code to wait for the threads to finish.
  // Otherwise main might run to the end
  // and kill the entire process when it exits.



  pthread_mutex_destroy(&g_buffer_mutex);

  return 0;
} /* main() */
