/* resched.c - resched, resched_cntl */

#include <xinu.h>
// #define DEBUG_CTXSW
struct	defer	Defer;
extern int32 Total_Tickets;
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/

	/* If rescheduling is deferred, record attempt and return */
	// print_ready_list();
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}
	
	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];

	// if(ptold->user_process == TRUE)
	// {
	// 	print_ready_list();
	// }
	// kprintf("user_process: %d, prname != prnull: %d\n", ptold->user_process, strcmp(ptold->prname, "prnull") != 0);
	if(ptold->user_process == FALSE && strcmp(ptold->prname, "prnull") != 0)		// To filter system process other than user_process and prnull
	{
		// kprintf("process pid going in: %d\n",currpid);
		if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
		}

		/* Force context switch to highest priority ready process */

		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/
		
		if(ptnew->prstate == PR_CURR && ptnew->user_process == TRUE)
		{
			// kprintf("pid; %d\n",currpid);
			ptnew->num_ctxsw++;
		}
		#ifdef DEBUG_CTXSW
			if(currpid != (ptold-proctab))
			{
				kprintf("ctxsw::%d-%d\n",(ptold-proctab),currpid);
			}
		
		#endif
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
		
		/* Old process returns here when resumed */

		return;

	}
	else if (ptold->user_process == TRUE && Total_Tickets > 0)
	{
		// kprintf("process pid going in: %d\n",currpid);
		
		Lottery_scheduler(ptold);
		// kprintf("returned to pavellion\n");
		return;
	}
	else
	{
		// kprintf("null else\n");
		// print_ready_list();
		
		if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */
		
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
		}

		/* Force context switch to highest priority ready process */

		currpid = dequeue(readylist);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;		/* Reset time slice for process	*/
		
		if(ptnew->prstate == PR_CURR && ptnew->user_process == TRUE)
		{
			// kprintf("pid; %d\n",currpid);
			ptnew->num_ctxsw++;
		}
		#ifdef DEBUG_CTXSW
			if(currpid != (ptold-proctab))
			{
				kprintf("ctxsw::%d-%d\n",(ptold-proctab),currpid);
			}
		
		#endif
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

		/* Old process returns here when resumed */
		// kprintf("here?\n");
		return;

	}
	
	return;
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
