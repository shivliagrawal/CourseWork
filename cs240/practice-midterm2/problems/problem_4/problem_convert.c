#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_DIGITS 32

int power(unsigned int i, unsigned int j) {
	int n=1;
	while(j > 0) {
		n *= i;
		j--;
	}

	return n;
}


// Problem (1/2)
/*****************************************************************************
 * TODO: convert a number from the given base to decimal
 *
 * Parameters: number -- the number you are converting an integer
 *             base   -- the base of the number you are given
 * 
 * Return: The number as an integer
 *
 * Return Type: int
*****************************************************************************/
int toInteger(char * number, int base) {
	int dec = 0;
	if (base > 10) {
		int a = 0;
		int size =0;
		char *or = number;
		while(*number) {
			size++;
			number++;
		}
		number--;
		while(number>=or) {
			int num;
			if (*number >= 65) {
				num = *number - 55;
			}
			else {
				num = *number - 48;
			}
			dec += num * power(base, a);
			number--;
			a++;
		}
		return dec;
	}

	int org = atoi(number);
	int a=0;
	while(org >0) {
		int num = org % 10;
		dec += num * power(base, a);
		org = org/10;
		a++;
	}

	return dec;
}

// Problem (2/2)
/*****************************************************************************
 * TODO: convert a number from the given base to decimal
 *
 * Parameters: number -- the number you are converting a string
 *             base   -- the base you are converting the numebr to
 * 
 * Return: The number as a string in base "base"
 *
 * Return Type: char *
*****************************************************************************/
char * toBase(int number, int base) {
	char array[32];
	for (int i=0; i<32; i++) {
		array[i] =0;
	}
	int b=0;
	while(number > 0) {
		int rem = number % base;
		if (rem > 9) {
			char num = rem + 55;
			array[b] = num;
		}
		else {
			char num = rem + 48;
			array[b] = num;
		}
		number = number / base;
		b++;
	}

	char *res = (char *)malloc((b+1) * sizeof(char));
	char *or = res;
	for (int j=b-1; j>= 0; j--) {
		*res = array[j];
		res++;
	}

	*res = '\0';

	return or;
}
