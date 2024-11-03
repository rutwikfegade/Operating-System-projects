#include <xinu.h>

// #define DEBUG
uint32 lock_count = 0;


void upgrade(pid32 pid,pi_lock_t *l)
{
    intmask 	mask; 
    mask = disable();
    int i;
    struct	procent	*prptr;
    uint32 max = 0;
    priority_holder = 0;
    priority_holder = proctab[l->lock_owner].prprio;
    #ifdef DEBUG
        kprintf("pid is %d with priority %d and lock owner is %d also priority_holder is %d and lock id is %d\n",pid,proctab[pid].prprio,l->lock_owner,priority_holder,l->lock_id);
    #endif
    bool8 check_flag = FALSE;
    for(i=0; i<NPROC; i++)
    {
        prptr = &proctab[i];
        #ifdef DEBUG
            kprintf("requested is %d and acquired is %d\n",prptr->requested,proctab[l->lock_owner].acquired);
        #endif
        if(prptr->requested == l->lock_id && prptr->requested != NOLOCK)
        {
            #ifdef DEBUG
                kprintf("PROC LOOP process %d with priority %d and comparison priority %d\n",i,prptr->prprio, proctab[l->lock_owner].prprio);
            #endif
            if(prptr->prprio > proctab[l->lock_owner].prprio)
            {
                max = prptr->prprio;
                check_flag = TRUE;
            }
        }
    }
    if(max > proctab[l->lock_owner].prprio)
    {
        proctab[l->lock_owner].prprio = max;
        kprintf("priority_change=P%d::%d-%d\n",l->lock_owner,priority_holder,proctab[l->lock_owner].prprio);
    } 
    restore(mask);
}
void pi_set_park(pi_lock_t *l)
{
    intmask	mask;
    mask = disable();
    l->set_park_flag = 1;
    restore(mask);
}

void pi_park(pi_lock_t *l)
{
    intmask	mask;
    struct	procent *prptr;
    prptr = &proctab[currpid];
    mask = disable();
    if(l->set_park_flag == 1)
    {
        l->set_park_flag = 0;
        prptr->prstate = PR_WAIT;
        resched(); 
    }
   
    restore(mask);
}

void pi_unpark(pid32 pid,pi_lock_t *l)
{
    intmask	mask;
    struct	procent *prptr;
    prptr = &proctab[pid];
    mask = disable();
    #ifdef DEBUG
        kprintf("owner is %d and its priority is %d\n",l->lock_owner, proctab[l->lock_owner].prprio);
        kprintf("inserting %d with priority %d\n",pid,prptr->prprio);
        kprintf("unparked pid: %d\n",currpid);
    #endif
    l->set_park_flag = 0;
    prptr->prstate = PR_READY;
    prptr->requested = NOPROC;
    insert(pid,readylist,prptr->prprio);
    resched();
    restore(mask);
}


syscall pi_initlock(pi_lock_t *l)
{
    l->flag = 0;
    l->guard = 0;
    l->lock_id = ++lock_count;
    l->lock_owner = NOPROC;
    l->lock_queue = newqueue();
    return OK;
}

syscall pi_lock(pi_lock_t *l)
{
    while(test_and_set(&l->guard,1)) sleepms(QUANTUM);
    if(l->flag == 0)
    {
        struct	procent	*prptr;
        prptr = &proctab[currpid];
        prptr->acquired = l->lock_id;
        l->acquired_priority = prptr->prprio;
        l->lock_owner = currpid;
        l->flag = 1;
        l->guard = 0;
    }
    else{
        enqueue(getpid(),l->lock_queue);
        struct	procent	*prptr;
        prptr = &proctab[currpid];
        prptr->requested = l->lock_id;
        upgrade(currpid,l);
        pi_set_park(l);
        l->guard = 0;
        pi_park(l);
    }
    l->guard = 0;
    return OK;
}

syscall pi_unlock(pi_lock_t *l)
{
    while(test_and_set(&l->guard,1)) sleepms(QUANTUM);
        if(isempty(l->lock_queue))
        {
            l->flag = 0;
        }
        else
        {
            #ifdef DEBUG
                kprintf("Process that called unlock is %d with priority %d\n",currpid,proctab[currpid].prprio);
                kprintf("lock priority %d with lock id %d\n",l->acquired_priority,l->lock_id);
                print_queue(l->lock_queue);
            #endif
            uint32 current_high_priority;
            struct	procent	*prptr;
            prptr = &proctab[currpid];
            current_high_priority = prptr->prprio;
            kprintf("priority_change=P%d::%d-%d\n",currpid,current_high_priority,l->acquired_priority);
            prptr->prprio = proctab[firstid(l->lock_queue)].prprio - 1;
            pid32 pid = dequeue(l->lock_queue);
            #ifdef DEBUG
                kprintf("Dequeued process %d has a priority of %d\n",pid,proctab[pid].prprio);
            #endif
            if(nonempty(l->lock_queue))
            {
                #ifdef DEBUG
                    kprintf("process %d lock_id %d process priority %d lock priority %d\n",pid,l->lock_id,proctab[pid].prprio,l->acquired_priority);
                #endif
                l->acquired_priority = proctab[pid].prprio;
                #ifdef DEBUG
                    kprintf("updates lock priority %d with lock id %d and process %d\n",l->acquired_priority,l->lock_id,pid);
                #endif
                kprintf("priority_change=P%d::%d-%d\n",pid,proctab[pid].prprio,current_high_priority);
                proctab[pid].prprio = current_high_priority;
            }
            pi_unpark(pid,l);
    }
    
    l->guard = 0;
    return OK;
}
