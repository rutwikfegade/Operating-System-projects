/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;
extern uint32 Total_Tickets;
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */

void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/



	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	ptold = &proctab[currpid];

	if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == FALSE)		// To get system process only
	{
		if (ptold->prstate == PR_CURR) { 
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		// kprintf("This is a system process with pid: %d and name: %s\n",currpid,ptold->prname);
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
		}


		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;	
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


		return;
	}
	else if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == TRUE)		// User process only
	{
		kprintf("Total_tickets: %d\n",Total_Tickets);
		if(Total_Tickets <= 0)
		{
			return;
		}
		if(ptold->tickets <= 0)
		{
			if (ptold->prstate == PR_CURR) 
			{ 
				// kprintf("Process inserted in userlist: %d\n",new_pid);
				ptold->prstate = PR_READY;
				kprintf("process %d being set to %d with %d tickets in resched else if\n",currpid,ptold->prstate,ptold->tickets);
				insert(currpid,user_list,ptold->tickets);
				print_queue(user_list);
			}
			currpid = dequeue(user_list);
			kprintf("dequeued process %d\n",currpid);
			print_ready_list();
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			preempt = QUANTUM;
			ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
			return;

		}
		kprintf("This is a user process with pid: %d and state: %d and tickets: %d\n",currpid,ptold->prstate,ptold->tickets);
		// print_ready_list();
		Lottery_scheduler(ptold);
		return;
	}
	else		// For null process
	{
		if(nonempty(readylist))
		{
			if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
			if (ptold->prprio > firstkey(readylist)) {
				return;
			}
		
			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->prprio);
			}

			currpid = dequeue(readylist);
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			preempt = QUANTUM;		
			ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
		}
		else if(nonempty(user_list))
		{
			Lottery_scheduler(ptold);
		}
		else
		{
			return;
		}

		return;
	}


}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
