#include <stdio.h>


void
count()
{
    int i = 0;
    while (i < 5) {
        printf("i=%d\n", i);
         i++;
    }
}

int x = 5;

int
main()
{
    printf("Hello world\n");
    x++;
    count();
}



