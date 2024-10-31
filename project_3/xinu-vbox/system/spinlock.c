#include <xinu.h>

syscall sl_initlock(sl_lock_t *mutex)
{
    mutex->flag = 0;
}

syscall sl_lock(sl_lock_t *mutex)
{
    // kprintf("Thread with pid: %d entered and has the mutex value of %d\n",currpid,mutex->flag);
    while(test_and_set(&mutex->flag,1));
    // kprintf("Thread %d acquired lock\n", currpid);
}

syscall sl_unlock(sl_lock_t *mutex)
{
    // kprintf("Thread with pid: %d tried to unlock\n",currpid);
    mutex->flag = 0;
    // kprintf("Thread %d released lock\n", currpid);
}