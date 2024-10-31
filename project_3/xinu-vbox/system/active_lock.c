#include <xinu.h>

uint32 lock_counter = 0;
uint32 counter = 0;
uint32 local_counter = 0;
void detect_deadlock(pid32 pid,al_lock_t *l)
{
    intmask 	mask; 
    mask = disable();
    // kprintf("Entering pid is %d with lock_id %d and lock_owner %d\n",pid,l->lock_id,l->lock_owner);
    int i,j,k;
    struct	procent	*Reference_proc;
    struct	procent	*Iterator_proc;
    uint32 flag_temp = 0;
    

    for(i=pid ; i<NPROC ; i++)
    {
        Reference_proc = &proctab[i];
        // kprintf("Reference process = %d with acquired %d and requested %d\n",i,Reference_proc->acquired,Reference_proc->requested);
        if(Reference_proc->acquired == NOLOCK || Reference_proc->requested == NOLOCK)
        {
            continue;
        }
        // kprintf("Reference process = %d with acquired %d and requested %d\n",i,Reference_proc->acquired,Reference_proc->requested); 
        for(j=0 ; j<NPROC ; j++)
        {
            Iterator_proc  = &proctab[j];
            // kprintf("Iterator process = %d with acquired %d and requested %d\n",j,Iterator_proc->acquired,Iterator_proc->requested);
            if(j == i || Iterator_proc->acquired == NOLOCK)
            {
                continue;
            }
           
            if((Reference_proc->requested == Iterator_proc->acquired) && (Reference_proc->acquired != Iterator_proc->acquired))
            {
                // kprintf("process that got selected %d and current process is %d \n",j,currpid);
                DeadLock_array[j] = j;
                local_counter++;
                // kprintf("acquired lock = %d and current lock id = %d\n",proctab[l->lock_owner].acquired,l->lock_id);
                // kprintf("counter = %d and local_counter = %d\n",counter,local_counter);
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
    while(test_and_set(&l->guard,1));
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
        // kprintf("entering pid is %d with lock %d\n",currpid,l->lock_id);
        detect_deadlock(currpid,l);
        set_park(l);
        l->guard = 0;
        park(l);
    }
    return OK;
}

syscall al_unlock(al_lock_t *l)
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
