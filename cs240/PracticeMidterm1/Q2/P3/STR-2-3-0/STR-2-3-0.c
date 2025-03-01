#include <stdlib.h>

/*
 *  NOTE NOTE NOTE NOTE NOTE
 *  You are not allowed to add any other libraries or library includes in addition
 *   to <stdlib.h> (if you believe you need it).
 */

/*	char *delChars(const char *s1, const char *s2)
 *
 *  Arguments:
 *		const char *s1 -- a pointer to a string
 *      const char *s2 -- a pointer to a string
 *
 *  Function return:
 *      char * -- a pointer to a string that contains the characters in "s1 that are
 *		 not in "s2".
 *
 *  Description:
 *		The function creates a new string with the characters from “s1” that
 *		 ignores any character that is contained in “s2”.
 *
 *	Example:
 *		delChars("Hello World", "Said George");  // returns "HllWl"
 */

char *delChars(const char *s1, const char *s2)
{
	int i = 0;
	char *delChars = malloc(200);
	int t=0;
	while(s1[i] != 0) {
		int read = 0;
		int j=0;
		while(s2[j] != 0) {
			if(s1[i] == s2[j]) {
			 	read = 1;
				break;
			}
			j++;
		}
		if (read) {
		} else {
			delChars[t] = s1[i];
			t++;
		}
		i++;

	}
	delChars[t+1] = 0;
	return delChars;
}
