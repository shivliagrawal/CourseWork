#include "part1.h"

#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// The producers add the characters in g_prod_str to the g_buffer, in order.
char *g_prod_str = "The greatest teacher, failure is.";

bounded_buffer g_buffer = {0};

// This mutex must be held whenever you use the g_buffer.
pthread_mutex_t g_buffer_mutex = PTHREAD_MUTEX_INITIALIZER;

// g_empty_sem is the number of characters in the g_buffer,
// that need to be emptied.
// Producers should signal g_empty_sem and consumers should wait on it.
sem_t g_empty_sem = {0};

// g_full_sem is the opposite of g_empty_sem. It is the number of
// unfilled slots in the g_buffer that need to be filled. Consumers
// should signal it, and producers should wait on it.
sem_t g_full_sem = {0};

/*
 * Produce the characters (that is, add them to the g_buffer) from g_prod_str,
 * in order. Signal consumers appropriately after each character.
 * Receive an ID via an (int *).
 */

void *producer(void *ptr) {
  int thread_id = *((int *) ptr);
  free(ptr);
  ptr = NULL;
  printf("Producer %d starting\n", thread_id);
  fflush(NULL);
  size_t len = strlen(g_prod_str);
  for (size_t i = 0; i < len; i++) {
    sem_wait(&g_full_sem);
    pthread_mutex_lock(&g_buffer_mutex);
    g_buffer.buf[g_buffer.tail] = g_prod_str[i];
    g_buffer.tail = (g_buffer.tail + 1) % BUF_SIZE;
    printf("Thread %d produced %c\n", thread_id, g_prod_str[i]);
    pthread_mutex_unlock(&g_buffer_mutex);
    sem_post(&g_empty_sem);
  }

  pthread_exit(0);
} /* producer() */

/*
 * Consume characters from the g_buffer. Stop after consuming the length of
 * g_prod_str, meaning that if an equal number of consumers and producers are
 * started, they will all exit. Signal producers appropriately of new free
 * space in the g_buffer. Receive an ID as an argument via an (int *).
 */

void *consumer(void *ptr) {
  int thread_id = *((int *) ptr);
  free(ptr);
  ptr = NULL;

  printf("Consumer %d starting\n", thread_id);
  fflush(NULL);

  size_t len = strlen(g_prod_str);

  for (size_t i = 0; i < len; i++) {
    sem_wait(&g_empty_sem);
    pthread_mutex_lock(&g_buffer_mutex);
    char c = g_buffer.buf[g_buffer.head];
    g_buffer.head = (g_buffer.head + 1) % BUF_SIZE;
    printf("Thread %d consumed %c\n", thread_id, c);
    pthread_mutex_unlock(&g_buffer_mutex);
    sem_post(&g_full_sem);
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
    fprintf(stderr, "Pass two arguments.\n");
    exit(1);
  }

  int numproducers = atoi(argv[1]);
  int numconsumers = atoi(argv[2]);

  pthread_mutex_init(&g_buffer_mutex, NULL);
  g_buffer.head = 0;
  g_buffer.tail = 0;
  sem_init(&g_empty_sem, 0, 0);
  sem_init(&g_full_sem, 0, BUF_SIZE);

  pthread_t producers[numproducers];
  for (int i = 0; i < numproducers; i++) {
    int *thread_id = malloc(sizeof(int));
    *thread_id = i;
    if (pthread_create(&producers[i], NULL, producer, (void *)thread_id) != 0) {
      perror("fail");
      exit(1);
    }
  }

  pthread_t consumers[numconsumers];
  for (int i = 0; i < numconsumers; i++) {
    int *thread_id = malloc(sizeof(int));
    *thread_id = i;
    if (pthread_create(&consumers[i], NULL, consumer, (void *)thread_id) != 0) {
      perror("fail");
      exit(1);
    }
  }

  for (int i = 0; i < numproducers; i++) {
    pthread_join(producers[i], NULL);
  }

  for (int i = 0; i < numconsumers; i++) {
    pthread_join(consumers[i], NULL);
  }

  pthread_mutex_destroy(&g_buffer_mutex);
  sem_destroy(&g_empty_sem);
  sem_destroy(&g_full_sem);

  return 0;
} /* main() */

