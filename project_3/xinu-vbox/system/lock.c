#include <xinu.h>

void set_park(lock_t *l)
{
    intmask	mask;
    mask = disable();
    // kprintf("Set parked pid: %d\n",currpid);
    l->set_park_flag = 1;
    restore(mask);
}

void park(lock_t *l)
{
    intmask	mask;
    struct	procent *prptr;
    prptr = &proctab[currpid];
    mask = disable();
    if(l->set_park_flag == 1)
    {
        // kprintf("parked pid: %d\n",currpid);
        l->set_park_flag = 0;
        prptr->prstate = PR_WAIT;
        resched(); 
    }
   
    restore(mask);
}

void unpark(pid32 pid,lock_t *l)
{
    intmask	mask;
    struct	procent *prptr;
    prptr = &proctab[pid];
    mask = disable();
    // kprintf("unparked pid: %d\n",currpid);
    l->set_park_flag = 0;
    prptr->prstate = PR_READY;
    insert(pid,readylist,prptr->prprio);
    resched();

    restore(mask);
}


syscall initlock(lock_t *l)
{
    l->flag = 0;
    l->guard = 0;
    l->lock_queue = newqueue();
    return OK;
}

syscall lock(lock_t *l)
{
    while(test_and_set(&l->guard,1));
    if(l->flag == 0)
    {
        // kprintf("pid: %d has the lock\n",currpid);
        l->flag = 1;
        l->guard = 0;
    }
    else{
        // kprintf("pid: %d getting queued\n",currpid);
        enqueue(getpid(),l->lock_queue);
        set_park(l);
        l->guard = 0;
        park(l);
    }
    return OK;
}

syscall unlock(lock_t *l)
{
    while(test_and_set(&l->guard,1));
    if(isempty(l->lock_queue))
    {
        l->flag = 0;
    }
    else
    {
        unpark(dequeue(l->lock_queue),l);
    }
    l->guard = 0;
    return OK;
}
