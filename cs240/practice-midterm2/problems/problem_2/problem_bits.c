#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Problem (1/4)
/******************************************************************************
 * TODO: Print the bits in bitmap as 0s and 1s
 * 
 * Parameters: bitmap -- print the binary representation of this number
 *
 * Return: void 
 *
 * Return Type: void
 *****************************************************************************/
void printBits(unsigned int bitmap)
{
	int array[32];
   	for (int a =0; a<32; a++) {
	        array[a] = 0;
	}
	int b = 0;
	while(bitmap != 0) {
		int rem = bitmap % 2;
		array[b] = rem;
		bitmap = bitmap / 2;
		b++;
	}
	for (int c =31; c>=0; c--) {
		printf("%d", array[c]);
	}
	printf("\n");
	printf("10987654321098765432109876543210\n");
}


// Problem (2/4)
/******************************************************************************
 * TODO: Set the ith bit in *bitmap with the value in bitValue (0 or 1)
 * 
 * Parameters: bitmap -- unsigned integer
 *	       i -- index of the bit to be changed
 *             bitValue -- change the ith bit to this value
 *
 * Return: void 
 *
 * Return Type: void
 *****************************************************************************/
void setBitAt( unsigned int *bitmap, int i, int bitValue ) 
{
        int array[32];
	for (int a=0; a<32; a++) {
		array[a] = 0;
	}
	int b = 0;
	while(*bitmap != 0) {
		int rem = *bitmap % 2;
		array[b] = rem;
		*bitmap = *bitmap/2;
		b++;
	}
	array[i] = bitValue;
	int n=0;
	int m =1;
	for (int q=0; q<32; q++) {
		if(array[q] == 1) {
			int e = q;
			while (e > 0) {
				 m *= 2;
				 e--;
			}
			n += m;
			m = 1;
		}
	}
	*bitmap = n;	
}

// Problem (3/4)
/******************************************************************************
 * TODO: Return the bit value (0 or 1) at the ith bit of the bitmap
 * 
 * Parameters: bitmap -- unsigned integer
 *	       i -- index of the bit to be retrieved
 *
 * Return: the ith bit value 
 *
 * Return Type: integer
 *****************************************************************************/
int getBitAt( unsigned int bitmap, unsigned int i) {
	int array[32];
	for(int i=0; i<32; i++) {
		array[i] = 0;
	}
	int b=0;
	while(bitmap != 0) {
		int rem = bitmap % 2;
		array[b] = rem;
		bitmap= bitmap/2;
		b++;
	}
	int n = array[i];
    	return n;
}

// Problem (4/4)
/******************************************************************************
 * TODO: Return the number of bits with the value 'bitValue'
 *	 If the bitValue is 0, return the number of 0s. If the bitValue is 1,
 *	 return the number of 1s.
 * 
 * Parameters: bitmap -- unsigned integer
 *	       bitValue -- count how many times this number, either 0 or 1,
 *	       appears in bitmap
 *
 * Return: count of 0s or 1s in bitmap 
 *
 * Return Type: integer
 *****************************************************************************/
int countBits( unsigned int bitmap, unsigned int bitValue) {
  	int array[32];
   	for(int i=0; i<32; i++) {
		array[i]=0;
	}
	int b=0;
	while(bitmap != 0) {
		int rem = bitmap %2;
		array[b] = rem;
		bitmap = bitmap/2;
		b++;
	}
	int one=0;
	int zero=0;
	for (int j=0; j<32; j++) {
		if(array[j] == 1) {
			one++;
		}
		if(array[j] == 0) {
			zero++;
		}
	}
	if (bitValue == 1) {
		return one;
	}
	if (bitValue== 0) {
		return zero;
	}

	return 0;
}

