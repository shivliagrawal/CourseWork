#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int power(unsigned int a, unsigned int b) {
        unsigned int n = 1;
        while(b > 0) {
                n *= a;
                b--;
        }
        return n;
}


int
main(int argc, char ** argv) {

        if (argc < 4) {
          printf("Usage:  convert <basefrom> <baseto> <number>\n");
          exit(1);
        }

        char * basefrom = argv[1];
	char * baseto = argv[2];
	char * number = argv[3];

	int from  = atoi(basefrom);
	int to = atoi(baseto);
	int n = atoi(number);
	int s;

	size_t length = strlen(number);

	int y1 =0;
	int x1 =0;
		
	printf("Number read in base %d: %s\n", from, number);
	
	if(from < 11 && to < 11 && from > 1 && to > 1) {
		int newNum = n;
		int newRem = 0;
		while (newNum > 0) {
			newRem = newNum % 10;
			newNum = newNum / 10;
			if (newRem >= from) {
				printf("Wrong digit in number.\n");
				exit(1);
			}
		}
		int qou;
		int rem;
		int t = 0;
		int * result = malloc(200);
		if(from != 10) {
			int x=0;
			int y =0;
			while(n > 0) {
				int j;
				j = n%10;
				n = (n - j)/10;
				y = j*(power(from,x)) + y;
				x++;
			}
			n = y;
		}
		printf("Converted to base 10: %d\n", n);
		qou = n;
		while(qou > 0) { 
			qou = n/to;
			rem = n%to;
			n = qou;
			result[t] = rem;
			//printf("%d/n", result[t]);
			t++;
		}
		printf("Converted to base %d: ", to);
		for(int i = t-1; i>=0; i--) {
			printf("%d", result[i]);
		}
		printf("\n");
	}
	
	else if((from > 10 && from < 26) || (to > 10 && to < 26)) {
		int rem;
		int qou;
	       	for(int i = length-1; i >= 0; i--) {
               		 if(number[i]> 64 && number[i] < 91) {
                       		 s = number[i] - 55;
                       		 y1 = s*(power(from,x1)) + y1;
                       		 x1++;
			 }
			 else {
				 s = number[i] - '0';
				 y1 = s*(power(from,x1)) + y1;
				 x1++;
			 }
		}
	       	printf("Converted to base 10: %d\n", y1);
		printf("Converted to base %d: ", to);
		qou = y1;
		int ar= 0;
		char *c = malloc(32);
                while(qou > 0) {
                        qou = y1/to;
                        rem = y1%to;
                        y1 = qou;
			char sh;
			if(rem > 9) {
				sh = rem + 55;
				c[ar] = sh;
				ar++;
			}
			else {
				sh = rem + 48;
				c[ar] = sh;
				ar++;
			}
                }
		for (int i = ar - 1; i >= 0; i--) {
			printf("%c", c[i]);
		}
		printf("\n");
	}
}



