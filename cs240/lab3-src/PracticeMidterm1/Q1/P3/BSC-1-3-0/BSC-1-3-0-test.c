#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int binaryToDecimal(char *num);

char *num; // 9 for '\0'
int res;

void test1()
{
	num = strdup("00010100");
	printf("%s ", num);

	if ((res = binaryToDecimal(num)) == -1) {
		printf("contains invalid characters.\n");
	} else {
		printf("= %d in base 10.\n", res);
	}

	free(num);
}

void test2()
{
	num = strdup("10101010");
	printf("%s ", num);

	if ((res = binaryToDecimal(num)) == -1) {
		printf("contains invalid characters.\n");
	} else {
		printf("= %d in base 10.\n", res);
	}

	free(num);
}

void test3()
{
	num = strdup("1a1b1c1d");
	printf("%s ", num);

	if ((res = binaryToDecimal(num)) == -1) {
		printf("contains invalid characters.\n");
	} else {
		printf("= %d in base 10.\n", res);
	}

	free(num);
}

void test4()
{
	num = strdup("11111111");
	printf("%s ", num);

	if ((res = binaryToDecimal(num)) == -1) {
		printf("contains invalid characters.\n");
	} else {
		printf("= %d in base 10.\n", res);
	}

	free(num);
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