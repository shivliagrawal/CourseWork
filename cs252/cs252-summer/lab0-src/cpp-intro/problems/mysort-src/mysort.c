#include "mysort.h"
#include <alloca.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//
// Sort an array of element of any type
// it uses "compFunc" to sort the elements.
// The elements are sorted such as:
//
// if ascending != 0
//   compFunc( array[ i ], array[ i+1 ] ) <= 0
// else
//   compFunc( array[ i ], array[ i+1 ] ) >= 0
//
// See test_sort to see how to use mysort.
//
void mysort( int n,                      // Number of elements
	     int elementSize,            // Size of each element
	     void * array,               // Pointer to an array
	     int ascending,              // 0 -> descending; 1 -> ascending
	     CompareFunction compFunc )  // Comparison function.
{
	// Add your code here
	char *arr= (char *)array; 
    	void *e = malloc(elementSize);  

    	for (int i = 0; i < n - 1; i++) {
        	for (int j = 0; j < n - 1 - i; j++) {
            		void *a = arr + j * elementSize;
            		void *b = arr + (j + 1) * elementSize;
            		int compare = compFunc(a, b);

			if ((ascending && compare > 0) || (!ascending && compare < 0)) {
                		memcpy(e, a, elementSize);
                		memcpy(a, b, elementSize);
                		memcpy(b, e, elementSize);
            		}
        	}
    	}	
}

