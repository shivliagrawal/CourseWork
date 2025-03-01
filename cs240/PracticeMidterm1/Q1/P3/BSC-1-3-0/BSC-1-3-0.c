#include <stdlib.h>

/*
 *  NOTE NOTE NOTE NOTE NOTE
 *  You are not allowed to add any other libraries or library includes in addition
 *   to <stdlib.h> (if you believe you need it).
 */

/*	int binaryToDecimal(char *num)
 *
 *  Arguments:
 *		char *num -- a pointer to a string of '1's and '0's
 *
 *  Returns:
 *		int -- the decimal equivalent of the given binary string, "num"
 *
 *  Description:
 *		The function determines and returns the decimal value of a binary
 *		 number.  The binary number is given as a string of eight character
 *		 '1's and '0's.  The function returns value is the decimal representation
 *		 of the given binary number string or (-1) if "num" contains character(s)
 *		 that are not a '1' or a '0'.
 *
 *	Note:
 *		The length of "num" will always be eight chars.
 *
 *	Error:
 *		If any character in "num" is not a '1' or '0', the function returns (-1)
 *
 *	Example:
 *		char *s = "00010100";
 *		binaryToDecimal(s);  // returns 20
 *
 *	Hint:
 *		decimalValue = the sum of '2 ^ (7 - i)' for each index 'i' in "num" that is a '1'.
 */

int myPow(int, int);

int binaryToDecimal(char *num)
{
	int dec=0;
	for(int i=0; i<8;i++) {
		if(num[i] != '0' && num[i] != '1') {
			return -1;
		}
	}
	int k;
	for(int j=0; j<8; j++) {
		if(num[j] == '1') {
			k = j;
			dec = myPow(2, (7-k)) + dec;
		}
	}
	return dec;
}

/* myPow:  returns base^power */
int myPow(int base, int power)
{
	int result = 1;

	while (power) {
		if (power & 1) {
			result *= base;
		}

		power >>= 1;
		base *= base;
	}

	return result;
}
