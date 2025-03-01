#include <xinu.h>

extern pid32 getpid(void);
extern pid32 getppid(pid32);
extern pri16 chprio(pid32, pri16);
extern syscall suspend(pid32);

void syscalldisp(void)
{
    uint32 scnum;
    asm volatile("movl %%edx, %0" : "=r" (scnum));

    switch(scnum) {
    case 20: {
            /* getpidtrap(): no arguments */
            pid32 ret = getpid();
            asm volatile("" : : "a" (ret));
            break;
        }
    case 21: {
            /* getppidtrap(pid): one argument */
            pid32 arg;
            asm volatile("movl 44(%%esp), %0" : "=r" (arg));
            pid32 ret = getppid(arg);
            asm volatile("" : : "a" (ret));
            break;
        }
    case 22: {
            /* chpriotrap(pid, newprio): two arguments */
            pid32 arg1;
            int tmp_arg2;
            asm volatile("movl 44(%%esp), %0" : "=r" (arg1));
            asm volatile("movl 48(%%esp), %0" : "=r" (tmp_arg2));
            int ret = (int) chprio(arg1, (pri16) tmp_arg2);
            asm volatile("" : : "a" (ret));
            break;
        }
    case 23: {
            /* suspendtrap(pid): one argument */
            pid32 arg;
            asm volatile("movl 44(%%esp), %0" : "=r" (arg));
            int ret = suspend(arg);
            asm volatile("" : : "a" (ret));
            break;
        }
    default: {
            int ret = -1;
            asm volatile("" : : "a" (ret));
            break;
        }
    }
}

