#include "part2.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define MIN_ARGS                  (3)
#define MAX_ARGS                  (5)
#define NITROGEN_ATOMS_IN_N2      (2)
#define OXYGEN_ATOMS_IN_O2        (2)
#define O2_MOLECULES_FOR_NO2      (2)
#define N2_MOLECULES_FOR_NO2      (1)
#define O2_MOLECULES_FOR_O3       (3)
#define MAX_ACTIVATION_ENERGY     (100)
#define RAND_MAX_VALUE            (100)

/* Global Variables */
int g_num_oxygen = 0;
int g_num_nitrogen = 0;
int g_num_o2 = 0;
int g_num_n2 = 0;

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond_atoms = PTHREAD_COND_INITIALIZER;
pthread_cond_t g_cond_molecules = PTHREAD_COND_INITIALIZER;

int g_oxy_created = 0;
int g_nitro_created = 0;
int g_o2_created = 0;
int g_n2_created = 0;
int g_ac_no2 = 0;
int g_ac_o3 = 0;

/* Function Declarations */
void *create_oxygen(void *ptr);
void *create_nitrogen(void *ptr);
void *create_n2(void *ptr);
void *create_o2(void *ptr);
void *create_no2(void *ptr);
void *create_o3(void *ptr);

/*
 * Function: create_oxygen
 * -----------------------
 * Creates oxygen atoms.
 *
 * ptr: Pointer to the number of oxygen atoms to create.
 */

void *create_oxygen(void *ptr) {
  int how_many = *((int *)ptr);
  free(ptr);
  ptr = NULL;

  for (int i = 0; i < how_many; i++) {
    pthread_mutex_lock(&g_mutex);
    g_num_oxygen++;
    printf("An atom of oxygen was created.\n");
    pthread_cond_broadcast(&g_cond_atoms);
    pthread_mutex_unlock(&g_mutex);
  }

  pthread_mutex_lock(&g_mutex);
  g_oxy_created = 1;
  pthread_cond_broadcast(&g_cond_atoms);
  pthread_mutex_unlock(&g_mutex);
  pthread_exit(0);
} /* create_oxygen() */

/*
 * Function: create_nitrogen
 * -------------------------
 * Creates nitrogen atoms.
 *
 * ptr: Pointer to the number of nitrogen atoms to create.
 */

void *create_nitrogen(void *ptr) {
  int how_many = *((int *)ptr);
  free(ptr);
  ptr = NULL;

  for (int i = 0; i < how_many; i++) {
    pthread_mutex_lock(&g_mutex);
    g_num_nitrogen++;
    printf("An atom of nitrogen was created.\n");
    pthread_cond_broadcast(&g_cond_atoms);
    pthread_mutex_unlock(&g_mutex);
  }

  pthread_mutex_lock(&g_mutex);
  g_nitro_created = 1;
  pthread_cond_broadcast(&g_cond_atoms);
  pthread_mutex_unlock(&g_mutex);
  pthread_exit(0);
} /* create_nitrogen() */

/*
 * Function: create_n2
 * -------------------
 * Creates N2 molecules from nitrogen atoms.
 *
 * ptr: Unused parameter.
 */

void *create_n2(void *ptr) {
  (void)(ptr);

  while (1) {
    pthread_mutex_lock(&g_mutex);
    while ((g_num_nitrogen < NITROGEN_ATOMS_IN_N2) && (!g_nitro_created)) {
      pthread_cond_wait(&g_cond_atoms, &g_mutex);
    }

    if (g_num_nitrogen >= NITROGEN_ATOMS_IN_N2) {
      g_num_nitrogen -= NITROGEN_ATOMS_IN_N2;
      g_num_n2++;
      printf("Two atoms of nitrogen combined to produce one molecule of N2.\n");
      pthread_cond_broadcast(&g_cond_molecules);
      pthread_mutex_unlock(&g_mutex);
    } else if ((g_nitro_created) && (g_num_nitrogen < NITROGEN_ATOMS_IN_N2)) {
      g_n2_created = 1;
      pthread_cond_broadcast(&g_cond_molecules);
      pthread_mutex_unlock(&g_mutex);
      break;
    } else {
      pthread_mutex_unlock(&g_mutex);
    }
  }

  pthread_exit(0);
} /* create_n2() */

/*
 * Function: create_o2
 * -------------------
 * Creates O2 molecules from oxygen atoms.
 *
 * ptr: Unused parameter.
 */

void *create_o2(void *ptr) {
  (void)(ptr);

  while (1) {
    pthread_mutex_lock(&g_mutex);
    while ((g_num_oxygen < OXYGEN_ATOMS_IN_O2) && (!g_oxy_created)) {
      pthread_cond_wait(&g_cond_atoms, &g_mutex);
    }

    if (g_num_oxygen >= OXYGEN_ATOMS_IN_O2) {
      g_num_oxygen -= OXYGEN_ATOMS_IN_O2;
      g_num_o2++;
      printf("Two atoms of oxygen combined to produce one molecule of O2.\n");
      pthread_cond_broadcast(&g_cond_molecules);
      pthread_mutex_unlock(&g_mutex);
    } else if ((g_oxy_created) && (g_num_oxygen < OXYGEN_ATOMS_IN_O2)) {
      g_o2_created = 1;
      pthread_cond_broadcast(&g_cond_molecules);
      pthread_mutex_unlock(&g_mutex);
      break;
    } else {
      pthread_mutex_unlock(&g_mutex);
    }
  }

  pthread_exit(0);
} /* create_o2() */

/*
 * Function: create_no2
 * --------------------
 * Creates NO2 molecules from N2 and O2 molecules,
 * considering activation energy.
 *
 * ptr: Unused parameter.
 */

void *create_no2(void *ptr) {
  (void)(ptr);

  unsigned int seed = time(NULL) ^ pthread_self();

  while (1) {
    pthread_mutex_lock(&g_mutex);
    while ((g_num_n2 < N2_MOLECULES_FOR_NO2) ||
           (g_num_o2 < O2_MOLECULES_FOR_NO2)) {
      if (((g_n2_created) && (g_num_n2 < N2_MOLECULES_FOR_NO2)) ||
          ((g_o2_created) && (g_num_o2 < O2_MOLECULES_FOR_NO2))) {
        pthread_mutex_unlock(&g_mutex);
        pthread_exit(0);
      }
      pthread_cond_wait(&g_cond_molecules, &g_mutex);
    }

    if ((g_n2_created) && (g_o2_created) &&
        (g_ac_no2 == MAX_ACTIVATION_ENERGY)) {
      pthread_mutex_unlock(&g_mutex);
      pthread_exit(0);
    }

    int random_value = rand_r(&seed) % RAND_MAX_VALUE;
    if (random_value < g_ac_no2) {
      pthread_mutex_unlock(&g_mutex);
      sched_yield();
      continue;
    }

    g_num_n2 -= N2_MOLECULES_FOR_NO2;
    g_num_o2 -= O2_MOLECULES_FOR_NO2;
    printf("One molecule of N2 and two molecules of O2 combined to produce "
           "two molecules of NO2.\n");

    pthread_cond_broadcast(&g_cond_molecules);
    pthread_mutex_unlock(&g_mutex);
  }

  pthread_exit(0);
} /* create_no2() */

/*
 * Function: create_o3
 * -------------------
 * Creates O3 molecules from O2 molecules,
 * considering activation energy.
 *
 * ptr: Unused parameter.
 */

void *create_o3(void *ptr) {
  (void)(ptr); /* Unused parameter */

  unsigned int seed = time(NULL) ^ pthread_self();

  while (1) {
    pthread_mutex_lock(&g_mutex);
    while (g_num_o2 < O2_MOLECULES_FOR_O3) {
      if ((g_o2_created) && (g_num_o2 < O2_MOLECULES_FOR_O3)) {
        pthread_mutex_unlock(&g_mutex);
        pthread_exit(0);
      }
      pthread_cond_wait(&g_cond_molecules, &g_mutex);
    }

    if ((g_o2_created) && (g_ac_o3 == MAX_ACTIVATION_ENERGY)) {
      pthread_mutex_unlock(&g_mutex);
      pthread_exit(0);
    }

    int random_value = rand_r(&seed) % RAND_MAX_VALUE;
    if (random_value < g_ac_o3) {
      pthread_mutex_unlock(&g_mutex);
      sched_yield();
      continue;
    }

    g_num_o2 -= O2_MOLECULES_FOR_O3;
    printf("Three molecules of O2 combined to produce two molecules of O3.\n");

    pthread_cond_broadcast(&g_cond_molecules);
    pthread_mutex_unlock(&g_mutex);
  }

  pthread_exit(0);
} /* create_o3() */

/*
 * Function: main
 * --------------
 * Main function to initialize threads and start the simulation.
 *
 * argc: Number of command-line arguments.
 * argv: Array of command-line arguments.
 */

int main(int argc, char **argv) {
  if ((argc < MIN_ARGS) || (argc > MAX_ARGS)) {
    fprintf(stderr, "Usage: %s <oxygen_atoms> <nitrogen_atoms> [<AENO2> "
                    "<AEO3>]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int oxygen_atoms = atoi(argv[1]);
  int nitrogen_atoms = atoi(argv[2]);

  /* Default activation energies */
  g_ac_no2 = 0;
  g_ac_o3 = 0;

  if (argc >= 4) {
    g_ac_no2 = atoi(argv[3]);
    if ((g_ac_no2 < 0) || (g_ac_no2 > MAX_ACTIVATION_ENERGY)) {
      fprintf(stderr, "Activation energy for NO2 must be between 0 and 100.\n");
      exit(EXIT_FAILURE);
    }
  }
  if (argc == 5) {
    g_ac_o3 = atoi(argv[4]);
    if ((g_ac_o3 < 0) || (g_ac_o3 > MAX_ACTIVATION_ENERGY)) {
      fprintf(stderr, "Activation energy for O3 must be between 0 and 100.\n");
      exit(EXIT_FAILURE);
    }
  }

  /* Seed the random number generator */
  srand(time(NULL));

  /* Create threads for atom creation */
  pthread_t oxygen_thread = {0};
  pthread_t nitrogen_thread = {0};
  int *o_atoms = malloc(sizeof(int));
  *o_atoms = oxygen_atoms;
  pthread_create(&oxygen_thread, NULL, create_oxygen, o_atoms);

  int *n_atoms = malloc(sizeof(int));
  *n_atoms = nitrogen_atoms;
  pthread_create(&nitrogen_thread, NULL, create_nitrogen, n_atoms);

  /* Create threads for molecule creation */
  pthread_t n2_thread = {0};
  pthread_t o2_thread = {0};
  pthread_create(&n2_thread, NULL, create_n2, NULL);
  pthread_create(&o2_thread, NULL, create_o2, NULL);

  /* Create threads for reactions */
  pthread_t no2_thread = {0};
  pthread_t o3_thread = {0};
  pthread_create(&no2_thread, NULL, create_no2, NULL);
  pthread_create(&o3_thread, NULL, create_o3, NULL);

  /* Wait for atom creation threads to finish */
  pthread_join(oxygen_thread, NULL);
  pthread_join(nitrogen_thread, NULL);

  /* Wait for molecule creation threads to finish */
  pthread_join(n2_thread, NULL);
  pthread_join(o2_thread, NULL);

  /* Wait for reaction threads to finish */
  pthread_join(no2_thread, NULL);
  pthread_join(o3_thread, NULL);

  /* Destroy mutex and condition variables */
  pthread_mutex_destroy(&g_mutex);
  pthread_cond_destroy(&g_cond_atoms);
  pthread_cond_destroy(&g_cond_molecules);

  exit(EXIT_SUCCESS);
} /* main() */

