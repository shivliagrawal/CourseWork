#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "review.h"

// Problem (1/4)
/******************************************************************************
 * TODO: Return a pointer to the first occurrence of the character 'c' in the 
 * string 's'. Return NULL if the character is not found.
 * 
 * Parameters: s -- string
 *             c -- search for this character in the string 's'
 *
 * Return: pointer to the first occurene of the character c in the string s
 *
 * Return Type: char pointer
 *****************************************************************************/
char *strchr(const char *s, int c) 
{
    // Write Your Code Here
    while(*s) {
	    if(*s == c) {
		    return s;
	    }
	    s++;
    }		
    return NULL;
}

// Problem (2/4)
/******************************************************************************
 * TODO: Return the maximum element in the array
 * 
 * Parameters: array -- an integer array
 *             n -- number of elements in the array
 *
 * Return: the maximum element in the array
 *
 * Return Type: integer
 *****************************************************************************/
int max_array( int * array, int n )
{
    // Write Your Code Here
    int max = array[0];
    for(int i=1; i<n; i++) {
	    if(array[i] > max) {
		    max = array[i];
	    }
    }

    return max;
}

// Problem (3/4)
/******************************************************************************
 * TODO: Return the 'ith' bit in 'bitmap' (either 0 or 1)
 * 
 * Parameters: bitmap -- unsigned integer
 *             ith -- index of the bit to be returned
 *
 * Return: the largest element in the array 
 *
 * Return Type: integer
 *****************************************************************************/
int get_bit_at( unsigned int bitmap, int ith )
{
    // Write Your Code Here
    int array[32];

    for(int i=0; i<32; i++) {
	    array[i] =0;
    }

    int b=0;
    while(bitmap != 0) {
	    int rem = bitmap%2;
	    bitmap = bitmap/2;
	    array[b] = rem;
	    b++;
    }

    return array[ith];
}

// Problem (4/4)
/******************************************************************************
 * TODO: Read from the file and store the entries in to a single linked list.
 * Return the list. Use the function llist_add( List * list, int value ) to
 * add entries to the list. 
 *
 * The file format will be:
 * value\n
 * value\n
 * value\n
 * ...
 *
 * Parameters: filename -- the name of the file you will be reading from
 *
 * Return: a singled linked list which contains the values read from the file
 *
 * Return Type: pointer to a single linked list
 *****************************************************************************/
List * llist_read( char * filename ) 
{
    // Write Your Code Here
    FILE *f = fopen(filename,"r");
    if(f == NULL) {
	    return NULL;
    }

    List * list = (List*)malloc(sizeof(List));
    llist_init(list);

    char *s = (char*)malloc(10 * sizeof(char));
    while ((s = fgets(s, 7, f)) != NULL) {
          int c;
          if (s != NULL) {
                c = atoi(s);
          }
          llist_add(list, c);
    }
    return list;

}
