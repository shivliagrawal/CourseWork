#include <xinu.h>
#include "prototypes.h"

pri16 chpriotrap(pid32 pid, pri16 newprio)
{
    pri16 retval;

    asm volatile (
        "movl $22, %%edx      \n\t"  
        "int $35              \n\t"  
        : "=a" (retval)       
        : "b" (pid), "c" (newprio) 
        : "edx", "esi", "edi", "memory"
    );

    return retval;
}

