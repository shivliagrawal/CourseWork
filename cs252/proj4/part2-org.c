#include "part2.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// The number of molecules of each type
// (O, N, O2, and N2)

int g_num_oxygen = 0;
int g_num_nitrogen = 0;
int g_num_o2 = 0;
int g_num_n2 = 0;

/*
 * Create oxygen atoms. The number of atoms to create is specified by the
 * argument, which is an int cast to (void *).
 */

void *create_oxygen(void *ptr) {
  int how_many = *((int *) ptr);
  free(ptr);
  ptr = NULL;

  // Remove the following line when implementing your code

  (void) (how_many);

  // Add your code to create how_many oxygen atoms

  // Print this for each atom created:

  printf("An atom of oxygen was created.\n");

  // Add your code to signal consumers of oxygen


  pthread_exit(0);
} /* create_oxygen() */

/*
 * Create nitrogen atoms. The number of atoms to create is specified by the
 * argument, which is an int cast to (void *).
 */

void *create_nitrogen(void *ptr) {
  int how_many = *((int *) ptr);
  free(ptr);
  ptr = NULL;

  // Remove the following line when implementing your code

  (void) (how_many);

  // Add your code to create how_many nitrogen atoms

  // Print this for each atom created:

  printf("An atom of nitrogen was created.\n");

  pthread_exit(0);
} /* create_nitrogen() */

/*
 * Form N2 molecules, by combining nitrogen atoms.
 */

void *create_n2(void *ptr) {

  // Remove the following line when implementing your code

  (void) (ptr);

  while (1) {
    // Add your code to wait for the proper signal.

    // Add your code to check if you can create an N2 molecule
    // and if so, adjust counts of N and N2

    printf("Two atoms of nitrogen combined to produce one molecule of N2.\n");

    // Add your code to signal consumers of N2

  }
} /* create_n2() */

/*
 * Form O2 molecules, by combining oxygen atoms.
 */

void *create_o2(void *ptr) {

  // Remove the following line when implementing your code

  (void) (ptr);

  while (1) {
    // Add your code to consume two O atoms and produce one O2 molecule

    printf("Two atoms of oxygen combined to produce one molecule of O2.\n");
  }
} /* create_o2() */

/*
 * Form NO2 molecules, by combining N2 and O2 molecules.
 */

void *create_no2(void *ptr) {

  // Remove the following line when implementing your code

  (void) (ptr);

  while (1) {
    // Add your code to consume one N2 molecule and two O2 molecules and
    // produce two NO2 molecules

    printf("One molecule of N2 and two molecules of O2 combined to "
           "produce two molecules of NO2.\n");
  }
} /* create_no2() */

/*
 * Form O3 molecules, by combining O2 molecules.
 */

void *create_o3(void *ptr) {

  // Remove the following line when implementing your code

  (void) (ptr);

  while (1) {
    // Add your code to consume three O2 molecules and produce two O3 molecules

    printf("Three molecules of O2 combined to produce two molecules of O3.\n");
  }
} /* create_o3() */


/*
 * Create threads to run each chemical reaction. Wait on all threads to avoid
 * any premature exit. The number of oxygen atoms to be created is specified by
 * the first command-line argument, and the number of nitrogen atoms with the
 * second.
 */

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Please pass two arguments.\n");
    exit(1);
  }

  // Remove the two following stataments when implementing your code

  (void)(argc);
  (void)(argv);

  // Add your code to create the threads.  Remember to allocate and pass the
  // arguments for create_oxygen and create_nitrogen.

  // Add your code to wait till threads are complete before main
  // continues. Unless we wait we run the risk of executing an exit which will
  // terminate the process and all threads before the threads have completed.

  exit(EXIT_SUCCESS);
}
