
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>

#include "rpn.h"
#include "nextword.h"
#include "stack.h"

double rpn_eval(char * fileName, double x) {
	double a, b, c;
	FILE *fd;

	fd = fopen(fileName, "r");
	char* word;
	stack_clear();
	int num = 0;
	int error = 0;
	while((word = nextword(fd)) != NULL) {
		if (num > 0 && (stack_is_empty) == 0) {
			printf("Stack Underflow\n");
			exit(1);
		}

		if (word[0] == '+') {
			a = stack_pop();
			b = stack_pop();
			c = a+b;
			stack_push(c);
		}

		else if (strcmp(word, "-")==0) {
			a = stack_pop();
			b = stack_pop();
			c = b-a;
			stack_push(c);
		}

		else if (word[0] == '*') {
			a = stack_pop();
			b = stack_pop();
			c = a*b;
			stack_push(c);
		}
		else if (word[0] == '/') {
			a = stack_pop();
			b = stack_pop();
			c = b/a;
			stack_push(c);
		}
		else if (strcmp(word, "cos")==0) {
			a = cos(stack_pop());
			stack_push(a);
		}
		else if (strcmp(word, "sin") ==0) {
			stack_push(sin(stack_pop()));
		}
		else if (strcmp(word, "log") ==0) {
			stack_push(log(stack_pop()));
		}
		else if (strcmp(word, "exp") ==0) {
			stack_push(exp(stack_pop()));
		}
		else if (strcmp(word, "pow") ==0) {
			stack_push(pow(stack_pop(), stack_pop()));
		}
		else if (word[0] == 'x') {
			stack_push(x);
		}
		else {
			double k = atof(word);
			stack_push(k);
		}
		num++;

	}
	if (stack_top() > 1) {
		printf("Elements remain in the stack\n");
		exit(1);
	}
	return stack_pop();

}


