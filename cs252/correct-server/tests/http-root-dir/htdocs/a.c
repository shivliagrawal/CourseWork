#include <time.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	const time_t val = 153072481L;
	struct tm *t = gmtime(&val);
	printf("t->tm_hour: %d\n", t->tm_hour);
	return 0;
}
