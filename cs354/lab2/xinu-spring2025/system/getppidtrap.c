#include <xinu.h>
#include "prototypes.h"

pid32 getppidtrap(pid32 pid)
{
    pid32 retval;

    asm volatile (
        "movl $21, %%edx      \n\t"  
        "int $35              \n\t"  
        : "=a" (retval)       
        : "b" (pid)           
        : "edx", "ecx", "esi", "edi", "memory"
    );

    return retval;
}

