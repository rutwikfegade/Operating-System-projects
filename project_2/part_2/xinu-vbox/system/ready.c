/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/

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
			kprintf("The ready process PID is %d and process name is %s\n",i,prptr->prname);
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

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	insert(pid, readylist, prptr->prprio);
	resched();

	return OK;
}
