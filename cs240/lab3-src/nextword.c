
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Separates the file into words
//

#define MAXWORD 200
char word[MAXWORD];
int wordLength;

// It returns the next word from fd.
// If there are no more more words it returns NULL. 
char *nextword(FILE * fd) {
	int c;
	
	// While it is not EOF read char
	while ((c = fgetc(fd)) != -1) {
		memset(word, 0, sizeof(word));
		wordLength = 0;
		int b = 0;
		// While it is not EOF and it is a non-space char
		while (c != -1 && c != ' ' && c != '\n' && c != '\t' && c != '\r') {
			word[wordLength] = c;
			wordLength++;
			c = fgetc(fd);
			b++;
		}
		if (b != 0) {
			return word;
		}
	}
		
	// Return null if there are no more words
	return NULL;
}

