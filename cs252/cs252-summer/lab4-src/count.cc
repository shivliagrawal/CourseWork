
/*
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

long count;

void increment(long ntimes )
{
	for ( int i = 0; i < ntimes; i++ ) {
		int c;

		c = count;
		c = c + 1;

		count = c;
	}
}

int main( int argc, char ** argv )
{
	long n = 10000000;
	pthread_t t1, t2;
        pthread_attr_t attr;

        pthread_attr_init( &attr );
        pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	printf("Start Test. Final count should be %ld\n", 2 * n );

	// Create threads
	pthread_create( &t1, &attr, (void * (*)(void *)) increment, 
			(void *) n);

	pthread_create( &t2, &attr, (void * (*)(void *)) increment, 
			(void *) n);

	// Wait until threads are done
	pthread_join( t1, NULL );
	pthread_join( t2, NULL );

	if ( count != 2 * n ) {
		printf("\n****** Error. Final count is %ld\n", count );
		printf("****** It should be %ld\n", 2 * n );
	}
	else {
		printf("\n>>>>>> O.K. Final count is %ld\n", count );
	}
}

*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

long count;
pthread_mutex_t count_mutex;

void increment(long ntimes)
{
    for (long i = 0; i < ntimes; i++) {
        pthread_mutex_lock(&count_mutex); // Lock the mutex
        count++;
        pthread_mutex_unlock(&count_mutex); // Unlock the mutex
    }
}

int main(int argc, char **argv)
{
    long n = 10000000;
    pthread_t t1, t2;
    pthread_attr_t attr;

    // Initialize the mutex
    pthread_mutex_init(&count_mutex, NULL);

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    printf("Start Test. Final count should be %ld\n", 2 * n);

    // Create threads
    pthread_create(&t1, &attr, (void *(*)(void *))increment, (void *)n);
    pthread_create(&t2, &attr, (void *(*)(void *))increment, (void *)n);

    // Wait until threads are done
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    if (count != 2 * n) {
        printf("\n****** Error. Final count is %ld\n", count);
        printf("****** It should be %ld\n", 2 * n);
    } else {
        printf("\n>>>>>> O.K. Final count is %ld\n", count);
    }

    // Destroy the mutex
    pthread_mutex_destroy(&count_mutex);

    return 0;
}

