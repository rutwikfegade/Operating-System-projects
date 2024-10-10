/* resched.c - resched, resched_cntl */

#include <xinu.h>
#define DEBUG_CTXSW
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

	// kprintf("process entering is %d\n",currpid);
	pid32 calling_pid = currpid;
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	ptold = &proctab[currpid];

	if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == FALSE)		// To get system process only
	{
		if(ptold->prstate == PR_CURR)
		{
			if(firstid(readylist) != 0)			//System
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
				ptold->num_ctxsw++;
				#ifdef DEBUG_CTXSW
					if(currpid != (ptold-proctab))
					{
						kprintf("1ctxsw::%d-%d\n",(ptold-proctab),currpid);
					}

				#endif
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


				return;
			}
			else if (nonempty(user_list))			// user
			{
				Lottery_scheduler(ptold,calling_pid);
			}
			else									// NULL
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
				ptold->num_ctxsw++;
				#ifdef DEBUG_CTXSW
					if(currpid != (ptold-proctab))
					{
						kprintf("1ctxsw::%d-%d\n",(ptold-proctab),currpid);
					}

				#endif
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


				return;
			}
		}
		else
		{
			if(firstid(readylist) != 0)			//System
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
				ptold->num_ctxsw++;
				#ifdef DEBUG_CTXSW
					if(currpid != (ptold-proctab))
					{
						kprintf("1ctxsw::%d-%d\n",(ptold-proctab),currpid);
					}

				#endif
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


				return;
			}
			else if (nonempty(user_list))			// user
			{
				Lottery_scheduler(ptold,calling_pid);
			}
			else									// NULL
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
				ptold->num_ctxsw++;
				#ifdef DEBUG_CTXSW
					if(currpid != (ptold-proctab))
					{
						kprintf("1ctxsw::%d-%d\n",(ptold-proctab),currpid);
					}

				#endif
				ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


				return;
			}
		}
		
	}
	else if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == TRUE && ptold->prstate == PR_CURR)		// User process only
	{
		// kprintf("This is a user process with pid: %d and state: %d and tickets: %d\n",currpid,ptold->prstate,ptold->tickets);
		// print_ready_list();
		Lottery_scheduler(ptold,calling_pid);
		return;
	}
	else		// For null process
	{
		if(nonempty(readylist))
		{
			// kprintf("in null checks readylist\n");
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
			ptold->num_ctxsw++;
			#ifdef DEBUG_CTXSW
				if(currpid != (ptold-proctab))
				{
					kprintf("2ctxsw::%d-%d\n",(ptold-proctab),currpid);
				}

			#endif
			ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
		}
		else if(nonempty(user_list))
		{
			// kprintf("in null checks userlist\n");
			Lottery_scheduler(ptold,calling_pid);
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