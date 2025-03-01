/*
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

volatile unsigned long lock = 0;

unsigned long 
test_and_set(volatile unsigned long * lock)
{
    unsigned long oldval = 1;
    asm volatile("xchgq %1,%0":"=r"(oldval):"m"(*lock),"0"(oldval):"memory");
    return oldval;
}

void
my_spin_lock( volatile unsigned long * lock )
{
}

void
my_spin_unlock( volatile unsigned long * lock )
{
}

long count;

void increment( long ntimes )
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


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

volatile unsigned long lock = 0;

unsigned long test_and_set(volatile unsigned long *lock)
{
    unsigned long oldval = 1;
    asm volatile("xchgq %1,%0"
                 : "=r"(oldval)
                 : "m"(*lock), "0"(oldval)
                 : "memory");
    return oldval;
}

void my_spin_lock(volatile unsigned long *lock)
{
    while (test_and_set(lock))
    {
        pthread_yield(); // Yield the CPU to other threads
    }
}

void my_spin_unlock(volatile unsigned long *lock)
{
    *lock = 0;
}

long count;

void increment(long ntimes)
{
    for (long i = 0; i < ntimes; i++)
    {
        my_spin_lock(&lock); // Lock
        count++;
        my_spin_unlock(&lock); // Unlock
    }
}

int main(int argc, char **argv)
{
    long n = 10000000;
    pthread_t t1, t2;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    printf("Start Test. Final count should be %ld\n", 2 * n);

    // Create threads
    pthread_create(&t1, &attr, (void *(*)(void *))increment, (void *)n);
    pthread_create(&t2, &attr, (void *(*)(void *))increment, (void *)n);

    // Wait until threads are done
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    if (count != 2 * n)
    {
        printf("\n****** Error. Final count is %ld\n", count);
        printf("****** It should be %ld\n", 2 * n);
    }
    else
    {
        printf("\n>>>>>> O.K. Final count is %ld\n", count);
    }

    return 0;
}
*/

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h> // Include the sched.h header for sched_yield()

volatile unsigned long lock = 0;

unsigned long
test_and_set(volatile unsigned long *lock)
{
    unsigned long oldval = 1;
    asm volatile("xchgq %1,%0":"=r"(oldval):"m"(*lock),"0"(oldval):"memory");
    return oldval;
}

void
my_spin_lock(volatile unsigned long *lock)
{
    while (test_and_set(lock)) {
//        sched_yield(); // Yield the CPU to avoid busy-waiting
    }
}

void
my_spin_unlock(volatile unsigned long *lock)
{
    *lock = 0;
}

long count;

void increment(long ntimes)
{
    for (long i = 0; i < ntimes; i++) {
        my_spin_lock(&lock); // Lock the spin lock
        int c = count;
        c = c + 1;
        count = c;
        my_spin_unlock(&lock); // Unlock the spin lock
    }
}

int main(int argc, char **argv)
{
    long n = 10000000;
    pthread_t t1, t2;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    printf("Start Test. Final count should be %ld\n", 2 * n);

    // Create threads
    pthread_create(&t1, &attr, (void * (*)(void *)) increment, (void *) n);
    pthread_create(&t2, &attr, (void * (*)(void *)) increment, (void *) n);

    // Wait until threads are done
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    if (count != 2 * n) {
        printf("\n****** Error. Final count is %ld\n", count);
        printf("****** It should be %ld\n", 2 * n);
    } else {
        printf("\n>>>>>> O.K. Final count is %ld\n", count);
    }

    return 0;
}

