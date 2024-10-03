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

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	ptold = &proctab[currpid];

	if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == FALSE)		// To get system process only
	{
		if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */
		// kprintf("This is a system process with pid: %d and name: %s\n",currpid,ptold->prname);
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
		}

		/* Force context switch to highest priority ready process */

		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

		/* Old process returns here when resumed */

		return;
	}
	else if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == TRUE)		// User process only
	{
		kprintf("Total_tickets: %d\n",Total_Tickets);
		if(Total_Tickets <= 0)
		{
			return;
		}
		kprintf("This is a user process with pid: %d and state: %d\n",currpid,ptold->prstate);
		// print_ready_list();
		Lottery_scheduler(ptold);
		return;
	}
	else		// For null process
	{
		// kprintf("null\n");
		if(nonempty(readylist))
		{
			if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
			if (ptold->prprio > firstkey(readylist)) {
				return;
			}
			// kprintf("This is a Null process with pid: %d and name: %s\n",currpid,ptold->prname);
			/* Old process will no longer remain current */

			ptold->prstate = PR_READY;
			insert(currpid, readylist, ptold->prprio);
			}

			/* Force context switch to highest priority ready process */

			currpid = dequeue(readylist);
			ptnew = &proctab[currpid];
			ptnew->prstate = PR_CURR;
			preempt = QUANTUM;		/* Reset time slice for process	*/
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

		/* Old process returns here when resumed */

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
