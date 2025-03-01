#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sortArray(int *numbers, unsigned int n, int descendFlag);

int n, flag;

void printArray(int arr[], unsigned n)
{
	int i;
	printf("{");

	for (i = 0; i < n; i++) {
		printf("%d%s", arr[i], (i == n - 1) ? "}" : ", ");
	}
}

void test1()
{
	int numbers[] = {14, 4, 16, 12};
	n = sizeof(numbers) / sizeof(numbers[0]);
	flag = 0;

	printf("Sorting ");
	printArray(numbers, n);
	printf(" in %s order...]\n", (flag) ? "descending" : "ascending");
	sortArray(numbers, n, flag);
	printArray(numbers, n);
}

void test2()
{

	int numbers[] = {1, 1, 2, 2, 3, 3, 500, 1, 3};
	n = sizeof(numbers) / sizeof(numbers[0]);
	flag = 0;

	printf("Sorting ");
	printArray(numbers, n);
	printf(" in %s order...]\n", (flag) ? "descending" : "ascending");
	sortArray(numbers, n, flag);
	printArray(numbers, n);
}

void test3()
{
	int numbers[] = {14, 4, 16, 12};
	n = sizeof(numbers) / sizeof(numbers[0]);
	flag = 1;

	printf("Sorting ");
	printArray(numbers, n);
	printf(" in %s order...]\n", (flag) ? "descending" : "ascending");
	sortArray(numbers, n, flag);
	printArray(numbers, n);
}

void test4()
{
	int numbers[] = {1, 1, 2, 2, 3, 3, 500, 1, 3};
	n = sizeof(numbers) / sizeof(numbers[0]);
	flag = 1;

	printf("Sorting ");
	printArray(numbers, n);
	printf(" in %s order...]\n", (flag) ? "descending" : "ascending");
	sortArray(numbers, n, flag);
	printArray(numbers, n);
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