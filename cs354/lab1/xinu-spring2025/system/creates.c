#include <xinu.h>
#include <stdarg.h>

pid32 creates(
    void     *funcaddr,   
    uint32    nargs,      
    ...
)
{
    intmask mask;          
    pid32   pid;           
    pri16   parent_prio;   
    va_list ap;           

    mask = disable();                      

    parent_prio = proctab[currpid].prprio;
    pri16 newprio = parent_prio + 1;

    va_start(ap, nargs);

   pid = create(
        funcaddr,
        PROCSTACKSZ,        
        newprio,            
        "NONAME",          
        nargs,              
        ap                 
    );

    va_end(ap);

    restore(mask);
    return pid;
}

