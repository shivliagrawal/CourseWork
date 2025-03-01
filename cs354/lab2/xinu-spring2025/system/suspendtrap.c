#include <xinu.h>
#include "prototypes.h"

syscall suspendtrap(pid32 pid)
{
    syscall retval;

    asm volatile (
        "movl $23, %%edx       \n\t"  
        "int $35               \n\t"  
        : "=a" (retval)              
        : "b" (pid)                 
        : "edx", "ecx", "esi", "edi", "memory"
    );

    return retval;
}

