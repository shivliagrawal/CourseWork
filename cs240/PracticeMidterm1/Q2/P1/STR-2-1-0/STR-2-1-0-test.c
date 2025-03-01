#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int mystrcmp(const char *s1, const char *s2);

char *s1, *s2;
int res;

void test1()
{
	s1 = strdup("Hello");
	s2 = strdup("World");

	if (!(res = mystrcmp(s1, s2))) {
		printf("\"%s\" is the same as \"%s\".\n", s1, s2);
	} else if (res == -1) {
		printf("\"%s\" is less than \"%s\".\n", s1, s2);
	} else if (res == 1) {
		printf("\"%s\" is greater than \"%s\".\n", s1, s2);
	} else {
		printf("Invalid return value.\n");
	}

	free(s1), s1 = NULL;
	free(s2), s2 = NULL;
}

void test2()
{
	s1 = strdup("Equal Strings");
	s2 = strdup("Equal Strings");

	if (!(res = mystrcmp(s1, s2))) {
		printf("\"%s\" is the same as \"%s\".\n", s1, s2);
	} else if (res == -1) {
		printf("\"%s\" is less than \"%s\".\n", s1, s2);
	} else if (res == 1) {
		printf("\"%s\" is greater than \"%s\".\n", s1, s2);
	} else {
		printf("Invalid return value.\n");
	}

	free(s1), s1 = NULL;
	free(s2), s2 = NULL;
}

void test3()
{
	s1 = strdup("DIFFERENT CASE");
	s2 = strdup("different case");

	if (!(res = mystrcmp(s1, s2))) {
		printf("\"%s\" is the same as \"%s\".\n", s1, s2);
	} else if (res == -1) {
		printf("\"%s\" is less than \"%s\".\n", s1, s2);
	} else if (res == 1) {
		printf("\"%s\" is greater than \"%s\".\n", s1, s2);
	} else {
		printf("Invalid return value.\n");
	}

	free(s1), s1 = NULL;
	free(s2), s2 = NULL;
}

void test4()
{
	s1 = strdup("\n\t\t\\\n\t\"");
	s2 = strdup("\n\t\t\\\n\t\"");

	if (!(res = mystrcmp(s1, s2))) {
		printf("\"%s\" is the same as \"%s\".\n", s1, s2);
	} else if (res == -1) {
		printf("\"%s\" is less than \"%s\".\n", s1, s2);
	} else if (res == 1) {
		printf("\"%s\" is greater than \"%s\".\n", s1, s2);
	} else {
		printf("Invalid return value.\n");
	}

	free(s1), s1 = NULL;
	free(s2), s2 = NULL;
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