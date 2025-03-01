#include <stdlib.h>

/*
 *  NOTE NOTE NOTE NOTE NOTE
 *  You are not allowed to add any other libraries or library includes in addition
 *   to <stdlib.h> (if you believe you need it).
 */

/*	void sortArray(int *numbers, unsigned int n, int descendFlag)
 *
 *  Arguments:
 *		int *numbers -- array of integers
 *      unsigned int n -- length of array "numbers"
 *      int descendFlag -- order of the sort (1) descending and ascending if
 *		 anything else
 *
 *  Description:
 *		The function sorts the array "numbers" of size "n" elements.  The
 *		 sorting is in descending order if the parameter "descendFlag" is set to
 *		 (1) and is in ascending order if it is anything else.
 *
 *  Example:
 *		int arr[] = {14, 4, 16, 12}
 *		sortArray(arr, 4, 0);  // [4, 12, 14, 16]
 *      sortArray(arr, 4, 1);  // [16, 14, 12, 4]
 */

void sortArray(int *numbers, unsigned int n, int descendFlag)
{
	int counter=1;
	while(counter < n) {
	 	for(int i = 0; i<n-1; i++) {
			for(int j = 1; j<n-i; j++) {
				if(descendFlag != 1) {
					int a;
					if(numbers[j-1]>numbers[j]) {
						a = numbers[j-1];
						numbers[j-1] = numbers[j];
						numbers[j] = a;
					}
				}
				else if (descendFlag ==1) {
					int b; 
					if(numbers[j]>numbers[j-1]) {
                                                b = numbers[j];
                                                numbers[j] = numbers[j-1];
                                                numbers[j-1] = b;
                                        }
				}
			}
		}
		counter++;
	}

}
