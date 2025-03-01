#include "part2.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// The number of atoms and molecules of each type
int g_num_oxygen = 0;
int g_num_nitrogen = 0;
int g_num_o2 = 0;
int g_num_n2 = 0;

// Mutex and condition variables for synchronization
pthread_mutex_t mutex;
pthread_cond_t cond_atoms;
pthread_cond_t cond_molecules;

// Flags to indicate when atom and molecule creation is complete
int oxygen_creation_complete = 0;
int nitrogen_creation_complete = 0;
int o2_creation_complete = 0;
int n2_creation_complete = 0;

void *create_oxygen(void *ptr) {
    int how_many = *((int *) ptr);
    free(ptr);
    ptr = NULL;

    for (int i = 0; i < how_many; i++) {
        // Simulate the creation of an oxygen atom
        pthread_mutex_lock(&mutex);
        g_num_oxygen++;
        printf("An atom of oxygen was created.\n");
        // Signal that an oxygen atom is available
        pthread_cond_broadcast(&cond_atoms);
        pthread_mutex_unlock(&mutex);
    }

    // Indicate that oxygen creation is complete
    pthread_mutex_lock(&mutex);
    oxygen_creation_complete = 1;
    pthread_cond_broadcast(&cond_atoms);
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
} /* create_oxygen() */

void *create_nitrogen(void *ptr) {
    int how_many = *((int *) ptr);
    free(ptr);
    ptr = NULL;

    for (int i = 0; i < how_many; i++) {
        // Simulate the creation of a nitrogen atom
        pthread_mutex_lock(&mutex);
        g_num_nitrogen++;
        printf("An atom of nitrogen was created.\n");
        // Signal that a nitrogen atom is available
        pthread_cond_broadcast(&cond_atoms);
        pthread_mutex_unlock(&mutex);
    }

    // Indicate that nitrogen creation is complete
    pthread_mutex_lock(&mutex);
    nitrogen_creation_complete = 1;
    pthread_cond_broadcast(&cond_atoms);
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
} /* create_nitrogen() */

void *create_n2(void *ptr) {
    (void) (ptr); // Unused parameter

    while (1) {
        pthread_mutex_lock(&mutex);
        // Wait until there are at least 2 nitrogen atoms
        while (g_num_nitrogen < 2 && !nitrogen_creation_complete) {
            pthread_cond_wait(&cond_atoms, &mutex);
        }

        if (g_num_nitrogen >= 2) {
            // Consume two nitrogen atoms to produce one N2 molecule
            g_num_nitrogen -= 2;
            g_num_n2++;
            printf("Two atoms of nitrogen combined to produce one molecule of N2.\n");
            // Signal that an N2 molecule is available
            pthread_cond_broadcast(&cond_molecules);
            pthread_mutex_unlock(&mutex);
        } else if (nitrogen_creation_complete && g_num_nitrogen < 2) {
            // No more nitrogen atoms will be created, and not enough to make N2
            n2_creation_complete = 1;
            pthread_cond_broadcast(&cond_molecules);
            pthread_mutex_unlock(&mutex);
            break;
        } else {
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(0);
} /* create_n2() */

void *create_o2(void *ptr) {
    (void) (ptr); // Unused parameter

    while (1) {
        pthread_mutex_lock(&mutex);
        // Wait until there are at least 2 oxygen atoms
        while (g_num_oxygen < 2 && !oxygen_creation_complete) {
            pthread_cond_wait(&cond_atoms, &mutex);
        }

        if (g_num_oxygen >= 2) {
            // Consume two oxygen atoms to produce one O2 molecule
            g_num_oxygen -= 2;
            g_num_o2++;
            printf("Two atoms of oxygen combined to produce one molecule of O2.\n");
            // Signal that an O2 molecule is available
            pthread_cond_broadcast(&cond_molecules);
            pthread_mutex_unlock(&mutex);
        } else if (oxygen_creation_complete && g_num_oxygen < 2) {
            // No more oxygen atoms will be created, and not enough to make O2
            o2_creation_complete = 1;
            pthread_cond_broadcast(&cond_molecules);
            pthread_mutex_unlock(&mutex);
            break;
        } else {
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_exit(0);
} /* create_o2() */

void *create_no2(void *ptr) {
    (void)(ptr); // Unused parameter

    while (1) {
        pthread_mutex_lock(&mutex);
        // Wait until there is at least 1 N2 and 2 O2 molecules
        while ((g_num_n2 < 1 || g_num_o2 < 2)) {
            // Check if no more N2 or O2 molecules will become available
            if ((n2_creation_complete && g_num_n2 < 1) ||
                (o2_creation_complete && g_num_o2 < 2)) {
                // No more reactions possible
                pthread_mutex_unlock(&mutex);
                pthread_exit(0);
            }
            pthread_cond_wait(&cond_molecules, &mutex);
        }

        // Consume molecules to produce NO2
        g_num_n2 -= 1;
        g_num_o2 -= 2;
        printf("One molecule of N2 and two molecules of O2 combined to "
               "produce two molecules of NO2.\n");

        // Signal that molecules have been consumed
        pthread_cond_broadcast(&cond_molecules);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
} /* create_no2() */

void *create_o3(void *ptr) {
    (void)(ptr); // Unused parameter

    while (1) {
        pthread_mutex_lock(&mutex);
        // Wait until there are at least 3 O2 molecules
        while (g_num_o2 < 3) {
            // Check if no more O2 molecules will become available
            if (o2_creation_complete && g_num_o2 < 3) {
                // No more reactions possible
                pthread_mutex_unlock(&mutex);
                pthread_exit(0);
            }
            pthread_cond_wait(&cond_molecules, &mutex);
        }

        // Consume three O2 molecules to produce two O3 molecules
        g_num_o2 -= 3;
        printf("Three molecules of O2 combined to produce two molecules of O3.\n");

        // Signal that molecules have been consumed
        pthread_cond_broadcast(&cond_molecules);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
} /* create_o3() */

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Please pass two arguments.\n");
        exit(1);
    }

    int oxygen_atoms = atoi(argv[1]);
    int nitrogen_atoms = atoi(argv[2]);

    // Initialize mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_atoms, NULL);
    pthread_cond_init(&cond_molecules, NULL);

    // Create threads for atom creation
    pthread_t oxygen_thread, nitrogen_thread;
    int *o_atoms = malloc(sizeof(int));
    *o_atoms = oxygen_atoms;
    pthread_create(&oxygen_thread, NULL, create_oxygen, o_atoms);

    int *n_atoms = malloc(sizeof(int));
    *n_atoms = nitrogen_atoms;
    pthread_create(&nitrogen_thread, NULL, create_nitrogen, n_atoms);

    // Create threads for molecule creation
    pthread_t n2_thread, o2_thread;
    pthread_create(&n2_thread, NULL, create_n2, NULL);
    pthread_create(&o2_thread, NULL, create_o2, NULL);

    // Create threads for reactions
    pthread_t no2_thread, o3_thread;
    pthread_create(&no2_thread, NULL, create_no2, NULL);
    pthread_create(&o3_thread, NULL, create_o3, NULL);

    // Wait for atom creation threads to finish
    pthread_join(oxygen_thread, NULL);
    pthread_join(nitrogen_thread, NULL);

    // Wait for molecule creation threads to finish
    pthread_join(n2_thread, NULL);
    pthread_join(o2_thread, NULL);

    // Wait for reaction threads to finish
    pthread_join(no2_thread, NULL);
    pthread_join(o3_thread, NULL);

    // Destroy mutex and condition variables
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_atoms);
    pthread_cond_destroy(&cond_molecules);

    exit(EXIT_SUCCESS);
}

