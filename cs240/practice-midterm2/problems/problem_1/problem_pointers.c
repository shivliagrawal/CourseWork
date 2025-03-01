#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Problem (1/4)
/******************************************************************************
 * TODO: Print the array.
 *       The format should be (array index)(colon)(array element)(newline) 
 * 
 * Parameters: n -- the number of elements in the array
 *             array -- a double array
 *
 * Return: void 
 *
 * Return Type: void
 *****************************************************************************/
void printArray(int n, double * array) {
	for (int i=0; i < n; i++) {
		printf("%d:%f\n", i,* array);
		array++;
	}
}
// Problem (2/4)
/******************************************************************************
 * TODO: Return the minimum element of array 
 * 
 * Parameters: array -- a double array
 *             n -- the number of elements in the array
 *
 * Return: minimum element in array 
 *
 * Return Type: double
 *****************************************************************************/
double minArray(int n, double * array) {
	int loc = 0;
	double min = *array;
	for(int i=0; i<n; i++) {
		if(*(array+1) < min) {
			min = *(array+1);
			array++;
		}	
		else if(min < *(array+1)) {
			array++;
		}
	}
	return min;
}

// Problem ( 3/4 ) 
/******************************************************************************
 * TODO: Reverse the given string 'str'. 
 * E.g. reverse_str("smile") should return "elims"
 * 
 * Parameters: str -- The given string to be reversed.
 *
 * Return: A pointer to str, str should be reversed 
 *
 * Return Type: char pointer
 *****************************************************************************/
char * reverse_str (char * str ) {
	int i;
	int cnt=0;
	char *temp = str;
	char *temp2 = str;
	while(*temp !='\0') {
		cnt++;
		temp++;
	}
	temp = str;
	for (int j=0; j<(cnt/2); j++) {
		char t = *str;
		int b = cnt - j - 1;
		*str = *(temp2 + b);
		*(temp2 + b) = t;
		str++;
		temp2 = temp;
	}
    	return temp2;
}

// Problem ( 4/4 ) 
/******************************************************************************
 * TODO: Determine if the string str2 is a permutation of str1. A permutation
 * is the rearranging of characters in a different order. 
 * E.g. the string "act" is a permutation of "cat" 
 *
 * Hint: count the occurences of each letter
 * 
 * Parameters: str1 -- The original string
 *	       str2 -- Determine if this string is a permutation of str1 
 *
 * Return: 1 if str2 is a permutation
 *         0 if str2 is not a permutation
 *
 * Return Type: integer
 *****************************************************************************/
int is_permutation ( char * str1, char * str2 ) {
       
	int a=0;
	int b=0;
	int c=0;
	int d=0;
        int e=0; 
	int f=0;
        int g=0; 
	int h=0;
	int i=0;
	int j=0; 
	int k=0;
	int l=0;
       	int m=0;
        int n=0;
        int o=0;
	int p=0;
        int q=0;
        int r=0;
        int s=0; 
	int t=0;
        int u=0;
        int v=0;
        int w=0;
        int x=0;
        int y=0;
        int z=0;
	int a1=0;
        int b1=0;
        int c1=0;
        int d1=0;
        int e1=0;
        int f1=0;
        int g1=0;
        int h1=0;
        int i1=0;
        int j1=0;
        int k1=0;
        int l1=0;
        int m1=0;
        int n1=0;
        int o1=0;
        int p1=0;
        int q1=0;
        int r1=0;
        int s1=0;
        int t1=0;
        int u1=0;
        int v1=0;
        int w1=0;
        int x1=0;
        int y1=0;
        int z1=0;
        char *temp = str1;
	char *temp3 = str1;
	char *temp2 = str2;
	char *temp4 = str2;
	int cnt = 0;
	int cnt2 = 0;
	while(*temp !='\0') {
		cnt++;
                temp++;
	}
	while(*temp2 !='\0') {
                cnt2++;
                temp2++;
        }
	if(cnt != cnt2) {
		return 0;
	}
	else {
		while(*temp3 != '\0') {
			int letter = *temp3;
			if (letter == 97) {
				a++;
			}
			if (letter == 98) {
                                b++;
                        }
		       	if (letter == 99) {
                                c++;
                        }
		       	if (letter == 100) {
                                d++;
                        }
		       	if (letter == 101) {
                                e++;
                        }
		       	if (letter == 102) {
                                f++;
                        }
		       	if (letter == 103) {
                                g++;
                        }
		       	if (letter == 104) {
                                h++;
                        }
		       	if (letter == 105) {
                                i++;
                        }
		       	if (letter == 106) {
                                j++;
                        }
		       	if (letter == 107) {
                                k++;
                        }
		       	if (letter == 108) {
                                l++;
                        }
		       	if (letter == 109) {
                                m++;
                        }
		       	if (letter == 110) {
                                n++;
                        }
		       	if (letter == 111) {
                                o++;
                        }
		       	if (letter == 112) {
                                p++;
                        }
		       	if (letter == 113) {
                                q++;
                        }
		       	if (letter == 114) {
                                r++;
                        }
		       	if (letter == 115) {
                                s++;
                        }
		       	if (letter == 116) {
                                t++;
                        }
		       	if (letter == 117) {
                                u++;
                        }
		       	if (letter == 118) {
                                v++;
                        }
		       	if (letter == 119) {
                                w++;
                        }
		       	if (letter == 120) {
                                x++;
                        }
		       	if (letter == 121) {
                                y++;
                        }
		       	if (letter == 122) {
                                z++;
			}
			temp3++;
		}
		while(*temp4 != '\0') {
			int letter = *temp4;
                        if (letter == 97) {
                                a1++;
                        }
                        if (letter == 98) {
                                b1++;
                        }
                        if (letter == 99) {
                                c1++;
                        }
                        if (letter == 100) {
                                d1++;
                        }
                        if (letter == 101) {
                                e1++;
                        }
                        if (letter == 102) {
                                f1++;
                        }
                        if (letter == 103) {
                                g1++;
                        }
                        if (letter == 104) {
                                h1++;
                        }
                        if (letter == 105) {
                                i1++;
                        }
                        if (letter == 106) {
                                j1++;
                        }
                        if (letter == 107) {
                                k1++;
                        }
                        if (letter == 108) {
                                l1++;
                        }
                        if (letter == 109) {
                                m1++;
                        }
                        if (letter == 110) {
                                n1++;
                        }
                        if (letter == 111) {
                                o1++;
                        }
			if (letter == 112) {
                                p1++;
                        }
                        if (letter == 113) {
                                q1++;
                        }
                        if (letter == 114) {
                                r1++;
                        }
                        if (letter == 115) {
                                s1++;
                        }
                        if (letter == 116) {
                                t1++;
                        }
                        if (letter == 117) {
                                u1++;
                        }
                        if (letter == 118) {
                                v1++;
                        }
                        if (letter == 119) {
                                w1++;
                        }
                        if (letter == 120) {
                                x1++;
                        }
                        if (letter == 121) {
                                y1++;
                        }
                        if (letter == 122) {
                                z1++;
                        }
			temp4++;
		}
		if(a == a1 && b == b1 && c == c1 && d == d1 && e == e1 && f == f1 && g == g1 && h == h1 && i == i1 && j == j1 && k == k1 && l == l1 && m == m1 && n == n1 && o == o1 && p == p1 && q == q1 && r == r1 && s == s1 && t == t1 && u == u1 && v == v1 && w == w1 && x == x1 && y == y1 && z == z1) {
		       return 1;
		}	
	}

    return 0;
}
