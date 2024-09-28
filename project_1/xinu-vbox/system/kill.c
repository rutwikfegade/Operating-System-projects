/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
void print_active_processes() 
{
    int i;
    kprintf("Active processes:\n");
    for (i = 0; i < NPROC; i++) 
	{
        if (proctab[i].prstate != PR_FREE) 
		{
            kprintf("Process %d: State=%d, Parent=%d\n", i, proctab[i].prstate, proctab[i].prparent);
        }
    }
	kprintf("\n");
}


syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	#ifdef DEBUG
		kprintf("pid that will be killed is %d\n",pid);
	#endif

	if(proctab[pid].user_process)
	{
		#ifdef DEBUG
			kprintf("Marked for Cascading Termination %d\n", pid);
			kprintf("active process = %d\n",prcount);
		#endif
		
		for(i=0;i<NPROC;i++)
		{
			if(proctab[i].prparent == pid && proctab[i].prstate != PR_FREE)
			{
				kill(i);
				
				
				kprintf("Killing child process %d of parent %d\n\n", i, pid);
				
				print_active_processes();
				sleepms(100);
			}
		}
		proctab[pid].prstate = PR_FREE;
		return OK;
	}
	
	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}
	freestk(prptr->prstkbase, prptr->prstklen);
	#ifdef DEBUG
		kprintf("Killed process %d\n", pid);
	#endif
	

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
