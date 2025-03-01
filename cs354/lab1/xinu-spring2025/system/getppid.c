#include <xinu.h>

pid32 getppid(pid32 pid)
{
    intmask mask;             
    pid32   ppid;            

    mask = disable();        

    if (pid < 0 || pid >= NPROC) {
        restore(mask);
        return SYSERR;        
    }

    if (proctab[pid].prstate == PR_FREE) {
        restore(mask);
        return SYSERR;       
    }

    ppid = proctab[pid].prparent;
    restore(mask);
    return ppid;
}

