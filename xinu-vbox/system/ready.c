/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	user_list;			/* Index of user ready list		*/

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
syscall  print_ready_list()
{
	int i;
	register struct procent *prptr;
	for(i=0;i<NPROC;i++)
	{
		prptr = &proctab[i];
		if(prptr->prstate == PR_READY)
		{
			kprintf("The ready process PID is %d and process name is %s with priority %d\n",i,prptr->prname,prptr->tickets);
		}
	}
}

status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */
	// kprintf("calling pid %d\n",pid);
	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	if(prptr->user_process == TRUE)
	{
		// kprintf("Inserting pid: %d\n",pid);
		insert_user_process(pid, user_list, prptr->tickets);
	}
	else
	{
		insert(pid, readylist, prptr->prprio);
	}
	
	
	resched();

	return OK;
}
