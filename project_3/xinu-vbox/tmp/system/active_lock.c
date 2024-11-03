#include <xinu.h>

// # define DEBUG
uint32 lock_counter = 0;
uint32 counter = 0;
uint32 local_counter = 0;



bool8 is_cycle(sid32  DeadLock_array[])
{
    pid32 first_pid,i,j;
    uint32 first_acquired_lock;
    struct	procent	*Reference;
    struct	procent	*Iterator;
    for(i=0;i<NPROC;i++)
    {
        if(DeadLock_array[i] != NOPROC)
        {
            first_pid = i;
            break;
        }
    }
    #ifdef DEBUG
        kprintf("first process found %d\n",first_pid);
    #endif
    first_acquired_lock = proctab[first_pid].acquired;
    for(i=0;i<NPROC; i++)
    {
        Reference = &proctab[i];
        if(Reference->acquired == NOLOCK || Reference->requested == NOLOCK)
        {
            continue;
        }
        for(j=0 ; j<NPROC ; j++)
        {
            Iterator  = &proctab[j];
            
            if(j == i || Iterator->acquired == NOLOCK)
            {
                continue;
            }
            #ifdef DEBUG
                kprintf("Iterator process = %d with first_acquired_lock %d and requested %d\n",j,first_acquired_lock,Iterator->requested);
            #endif
            if(first_acquired_lock == Iterator->requested && DeadLock_array[j] != NOPROC)
            {
                #ifdef DEBUG
                    kprintf("Process is %d first_acquired_lock is %d\n",j,first_acquired_lock);
                #endif
                return TRUE;
            }
        }

        
    }
    return FALSE;

}


void detect_deadlock(pid32 pid,al_lock_t *l)
{
    intmask 	mask; 
    mask = disable();
    #ifdef DEBUG
        kprintf("Entering pid is %d with lock_id %d and lock_owner %d\n",pid,l->lock_id,l->lock_owner);
    #endif
    int i,j,k;
    struct	procent	*Reference_proc;
    struct	procent	*Iterator_proc;
    uint32 flag_temp = 0;
    

    for(i=pid ; i<NPROC ; i++)
    {
        Reference_proc = &proctab[i];
        #ifdef DEBUG
           kprintf("Reference process = %d with acquired %d and requested %d\n",i,Reference_proc->acquired,Reference_proc->requested);
        #endif
        if(Reference_proc->acquired == NOLOCK || Reference_proc->requested == NOLOCK)
        {
            continue;
        }
        #ifdef DEBUG
            kprintf("Reference process = %d with acquired %d and requested %d\n",i,Reference_proc->acquired,Reference_proc->requested); 
        #endif
        for(j=0 ; j<NPROC ; j++)
        {
            Iterator_proc  = &proctab[j];
            #ifdef DEBUG
                kprintf("Iterator process = %d with acquired %d and requested %d\n",j,Iterator_proc->acquired,Iterator_proc->requested);
            #endif
            if(j == i || Iterator_proc->acquired == NOLOCK)
            {
                continue;
            }
            #ifdef DEBUG
                kprintf("Iterator process = %d with acquired %d and requested %d\n",j,Iterator_proc->acquired,Iterator_proc->requested);
            #endif
            if((Reference_proc->requested == Iterator_proc->acquired) && (Reference_proc->acquired != Iterator_proc->acquired))
            {
                #ifdef DEBUG
                    kprintf("process that got selected %d and current process is %d \n",j,currpid);
                #endif
                if(DeadLock_array[j] == NOPROC)
                {
                    DeadLock_array[j] = j;
                    local_counter++;
                    #ifdef DEBUG
                        kprintf("process that got selected %d and current process is %d \n", j, currpid);
                        kprintf("counter = %d and local_counter = %d\n", counter, local_counter);
                    #endif
                }
                #ifdef DEBUG
                    kprintf("acquired lock = %d and current lock id = %d\n",proctab[l->lock_owner].acquired,l->lock_id);
                    kprintf("counter = %d and local_counter = %d\n",counter,local_counter);
                #endif
                if(counter == local_counter)
                {
                    uint32 temp_counter = counter;
                    kprintf("deadlock_detected=");
                    for(k=0 ; k<NPROC ; k++)
                    {
                        if(DeadLock_array[k] != NOPROC)
                        {
                            temp_counter--;
                            kprintf("P%d",DeadLock_array[k]);
                            if(temp_counter > 0)
                            {
                                kprintf("-");
                            }
                            DeadLock_array[k] = NOPROC;
                            proctab[k].acquired = NOPROC;
                            proctab[k].requested = NOPROC;
                            counter = 0;
                            local_counter = 0;
                        }
                    }
                    kprintf("\n");
                    flag_temp = 1;
                    break;
                }

            }


        }
        if (flag_temp)
        {
            break;
        }

    }
    restore(mask);
}

void al_set_park(al_lock_t *l)
{
    intmask	mask;
    mask = disable();
    l->set_park_flag = 1;
    restore(mask);
}

void al_park(al_lock_t *l)
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

void al_unpark(pid32 pid,al_lock_t *l)
{
    intmask	mask;
    struct	procent *prptr;
    prptr = &proctab[pid];
    mask = disable();;
    l->set_park_flag = 0;
    prptr->prstate = PR_READY;
    insert(pid,readylist,prptr->prprio);
    resched();

    restore(mask);
}



syscall al_initlock(al_lock_t *l)
{
    l->flag = 0;
    l->guard = 0;
    l->lock_id = ++lock_counter;
    l->lock_owner = NOPROC;
    l->lock_queue = newqueue();
    return OK;
}

syscall al_lock(al_lock_t *l)
{
    while(test_and_set(&l->guard,1)) sleepms(QUANTUM);
    if(l->flag == 0)
    {
        struct	procent	*prptr;
        prptr = &proctab[currpid];
        prptr->acquired = l->lock_id;
        l->lock_owner = currpid;
        counter++;
        counter_array[currpid] = 1;
        l->flag = 1;
        l->guard = 0;
    }
    else{
        enqueue(getpid(),l->lock_queue);
        struct	procent	*prptr;
        prptr = &proctab[currpid];
        prptr->requested = l->lock_id;
        detect_deadlock(currpid,l);
        al_set_park(l);
        l->guard = 0;
        al_park(l);
    }
    return OK;
}

syscall al_unlock(al_lock_t *l)
{
    while(test_and_set(&l->guard,1)) sleepms(QUANTUM);
    if(isempty(l->lock_queue))
    {
        l->flag = 0;
    }
    else
    {
        al_unpark(dequeue(l->lock_queue),l);
    }
    l->guard = 0;
    return OK;
}


bool8 al_trylock(al_lock_t *l) 
{
    while(test_and_set(&l->guard,1)) sleepms(QUANTUM);
    if(l->flag == 0)
    {
        l->flag = 1;
        l->guard = 0;
        return TRUE;
    }
    else{
        l->guard = 0;;
        return FALSE;
    }
    return OK;
} 