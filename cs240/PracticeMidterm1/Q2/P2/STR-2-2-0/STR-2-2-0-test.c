#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void toLowerStr(char *s);

char *s;

void test1()
{
	s = strdup("Hello");

	printf("Converting \"%s\" to lower-case...\n", s);
	toLowerStr(s);
	printf("Result = \"%s\".\n", s);
	free(s), s = NULL;
}

void test2()
{
	s = strdup("HELLO WORLD!");

	printf("Converting \"%s\" to lower-case...\n", s);
	toLowerStr(s);
	printf("Result = \"%s\".\n", s);
	free(s), s = NULL;
}

void test3()
{
	s = strdup("SYMBOLS:!@#$^&*(*)");

	printf("Converting \"%s\" to lower-case...\n", s);
	toLowerStr(s);
	printf("Result = \"%s\".\n", s);
	free(s), s = NULL;
}

void test4()
{
	s = strdup("\t\tHELLO WORLD!\t\t");

	printf("Converting \"%s\" to lower-case...\n", s);
	toLowerStr(s);
	printf("Result = \"%s\".\n", s);
	free(s), s = NULL;
}

int main(int argc, char **argv)
{
	char *test;

	if (argc < 2) {
		printf("Usage: mytest <test#>\n");
		exit(1);
	}

	test = argv[1];
	printf("Running %s\n", test);

	if (strcmp(test, "test1") == 0) {
		test1();
	} else if (strcmp(test, "test2") == 0) {
		test2();
	} else if (strcmp(test, "test3") == 0) {
		test3();
	} else if (strcmp(test, "test4") == 0) {
		test4();
	} else {
		printf("Test not found!\n");
		exit(1);
	}

	return 0;
}
