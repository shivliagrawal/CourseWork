#include <stdio.h>
#include <stdlib.h>

void printNums() {
	int a=0;
	while(TRUE) {
		int input;
		if((scanf("%d", &input))==1) {
			if (a != 0) {
				printf(",");
			}
			else {
				printf("{");
			}
			printf("%d", input);
			a++;
			if(getchar() == '\n') {
				break;
			}
		} else {
			printf("No integers were provided.\n");
			return;
		}
	}
	printf("}");
}
