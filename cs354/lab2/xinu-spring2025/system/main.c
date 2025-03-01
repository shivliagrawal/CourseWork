/*  main.c  - main */

#include <xinu.h>



process main(void)
{
    
	kprintf("Testing getpidtrap()...\n");
    pid32 mypid = getpidtrap();
    kprintf("Returned pid: %d\n", mypid);

    kprintf("Testing getppidtrap()...\n");
    pid32 parent = getppidtrap(getpid());
    kprintf("Returned parent pid: %d\n", parent);

    kprintf("Testing chprio()...\n");
    pri16 old_prio = chpriotrap(getpid(), 5);
    kprintf("Returned old priority: %d\n", old_prio);


    return OK;
}


