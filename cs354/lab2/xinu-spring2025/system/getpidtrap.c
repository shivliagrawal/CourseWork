#include <xinu.h>
#include "prototypes.h"

pid32 getpidtrap(void)
{
    pid32 retval;

    asm volatile (
        "movl $20, %%edx      \n\t"  
        "int $35              \n\t"  
        : "=a" (retval)       
        :                     
        : "edx", "ecx", "ebx", "esi", "edi", "memory"
    );

    return retval;
}

