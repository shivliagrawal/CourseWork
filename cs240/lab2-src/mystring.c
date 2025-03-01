
#include <stdlib.h>
#include "mystring.h"

// Type "man string" to see what every function expects.

int mystrlen(char * s) {
	int num = 0;
	while(* s!=0) {
		num++;
		s++;
	}
	return num;
}

char * mystrcpy(char * dest, char * src) {

	if (dest == NULL) {
        return NULL;
    }

    char *ptr = dest;

    while (*src != '\0')
    {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';

    return ptr;
}


char * mystrcat(char * dest, char * src) {
	int i,j;
	for(i = 0; dest[i] != '\0'; i++);
	for(j = 0; src[j] != '\0'; j++) {
		dest[i+j] = src[j];
	}
	dest[i+j] = '\0';

        return dest; 
}


int mystrcmp(char * s1, char * s2) {
	 while (*s1)
    {
        if (*s1 != *s2) {
            break;
        }

        s1++;
        s2++;
    }


	int x = *(const unsigned char*)s1 - *(const unsigned char*)s2;

	if(x >0) {
		return 1;
	}
	if(x<0) {
		return -1;
	}

	return 0;

}

char * mystrstr(char * hay, char * needle) {
	char * temp = hay;
	char * temp2 = needle;
	int l1 =0;
	int l2 =0;
	while(temp[l1]) {
		l1++;
	}	
	while(temp2[l2]) {
		l2++;
	}
	int cnt=0;
	temp2 = needle;
	while(*hay != '\0') {
		temp = hay;
		if(*hay == *needle) {
			char* ret = hay;
			for(int i=0; i<l2; i++) {
				hay++;
				needle++;
				if(*hay == *needle) {
					cnt++;
				}
			}
			needle = temp2;
			hay = temp;
			if(cnt == (l2-1)) {
				return ret;
			}	
		}
		hay++;
	}

	return NULL;

}

char * mystrdup(char * s) {
	char * a;
	char * b;
	int l=0;
	while(s[l]) {
		l++;
	}

	a = malloc(l + 1);
	b = a;
	while(* s) {
		*b++ = * s++; 
	}
	* b = '\0';
	return a;
}

char * mymemcpy(char * dest, char * src, int n)
{
	char *src1 = (char *)src;
	char *dest1 = (char *)dest;
	for (int i =0; i < n; i++) {
		dest1[i] = src1[i];
	}
	return NULL;
}

