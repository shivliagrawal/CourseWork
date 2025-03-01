#include <stdio.h>
#include <stdlib.h>


unsigned int power(unsigned int i, unsigned int j) {
	unsigned int n = 1;
	while (j > 0) {
		n *= i;
		j--;
	}
	return n;
}

// It prints the bits in bitmap as 0s and 1s.
void printBits(unsigned int bitmap)
{
	int array[32];
	for (int a = 0; a < 32; a++) {
		array[a] = 0;
	}
	int b = 0;
	while(bitmap != 0) {
		int rem = bitmap % 2;
		bitmap = bitmap / 2;
		array[b] = rem;
		b++;
	}
	for (int c = 31; c >= 0; c--) {
		printf("%d", array[c]);
	}
	printf("\n");
	int x =1;
	for (int d = 0; d < 32; d++) {
		printf("%d", x);
		x--;
		if(x<0) {
			x=9;
		}
	}
	printf("\n");


	// Add your code here
}


// Sets the ith bit in *bitmap with the value in bitValue (0 or 1)
void setBitAt( unsigned int *bitmap, int i, int bitValue ) {
	int bitarray[32];
	for (int z = 0; z <32; z++) {
		bitarray[z] = 0;
	}
	int s =0;
	while (*bitmap !=0) {
		int rem = *bitmap % 2;
		*bitmap = *bitmap/2;
		bitarray[s] = rem;
		s++;
	}

	bitarray[i] = bitValue;
	unsigned int n = 0;
	for(int q = 0; q < 32; q++) {
		if(bitarray[q] ==1) {
			n += power(2,q);
		}
	}
	*bitmap = n;
	// Add your code here
}

// It returns the bit value (0 or 1) at bit i
int getBitAt( unsigned int bitmap, unsigned int i) {
	// Add your code here
	int bit[32];
	for (int w = 0; w < 32; w++) {
		bit[w] = 0;
	}
	int t =0;
	while (bitmap != 0) {
		int rem = bitmap%2;
		bitmap = bitmap /2;
		bit[t] = rem;
		t++;
	}
	return bit[i];


}

// It returns the number of bits with a value "bitValue".
// if bitValue is 0, it returns the number of 0s. If bitValue is 1, it returns the number of 1s.
int countBits( unsigned int bitmap, unsigned int bitValue) {
	// Add your code here
	int bit[32];
	for(int f = 0; f < 32; f++) {
		bit[f] = 0;
	}
	int t = 0;
	while (bitmap !=0) {
		int rem = bitmap%2;
		bitmap = bitmap/2;
		bit[t] = rem;
		t++;
	}
	int count = 0;
	for(int f = 0; f < 32; f++) {
		if(bit[f] == bitValue) {
			count++;
		}
	}
	return count;

}

// It returns the number of largest consecutive 1s in "bitmap".
// Set "*position" to the beginning bit index of the sequence.
int maxContinuousOnes(unsigned int bitmap, int * position) {
	// Add your code here
	int bit[32];
	for (int j = 0; j < 32; j++) {
		bit[j] = 0;
	}

	int b = 0;
        while (bitmap !=0) {
                int rem = bitmap%2;
                bitmap = bitmap/2;
                bit[b] = rem;
                b++;
        }


	int p = 0;
	int count = 0;
	int maxp = 0;
	int maxcnt = 0;
	for(int k = 0; k < 32; k++) {
		if(bit[k] == 1) {
			if(k == 0 || bit[k-1] == 0 ) {
				p = k;
			}
			count++;
		} else {
			if(count > maxcnt) {
				maxcnt = count;
				maxp = p;
			}
			count = 0;
			p = 0;
		}
	}
	 *position = maxp;
      	 return maxcnt;

}



