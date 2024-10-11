/* resched.c - resched, resched_cntl */

#include <xinu.h>
#define DEBUG_CTXSW
struct	defer	Defer;
pid32 new_pid;
extern uint32 Total_Tickets;
struct procent *ptold;	/* Ptr to table entry for old process	*/
struct procent *ptnew;	/* Ptr to table entry for new process	*/
// extern int32 Total_Tickets;
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void system_to_other(struct procent *ptold, pid32 calling_pid)
{
	// print_ready_list();
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
		ptnew->num_ctxsw++;
		#ifdef DEBUG_CTXSW
			if(currpid != calling_pid)
			{
				kprintf("1ctxsw::%d-%d\n",calling_pid,currpid);
			}

		#endif
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


		return;
}

void user_to_system(struct procent *ptold, pid32 calling_pid)
{
	// print_ready_list();
	if(Total_Tickets <= ptold->tickets)
	{
		return;
	}
	if (ptold->prstate == PR_CURR) { 
	// kprintf("This is a system process with pid: %d and name: %s\n",currpid,ptold->prname);
	ptold->prstate = PR_READY;
	insert_user_process(currpid, user_list, ptold->tickets);
	}
	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;	
	ptnew->num_ctxsw++;
	#ifdef DEBUG_CTXSW
		if(currpid != calling_pid)
		{
			kprintf("2ctxsw::%d-%d\n",calling_pid,currpid);
		}

	#endif
	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


	return;
}

void system_to_user(struct procent *ptold, pid32 calling_pid)
{
	// print_ready_list();
	if (ptold->prstate == PR_CURR) { 
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		// kprintf("This is a system process with pid: %d and name: %s\n",currpid,ptold->prname);
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
		}
		
		new_pid = Find_winner();
		if(new_pid == currpid)
		{
			return;
		}
		// print_queue(user_list);
		dequeue_user_list(new_pid,user_list);
		// print_queue(user_list);
		currpid = new_pid;
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;	
		ptnew->num_ctxsw++;
		#ifdef DEBUG_CTXSW
			if(currpid != calling_pid)
			{
				kprintf("3ctxsw::%d-%d\n",calling_pid,currpid);
			}

		#endif
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


		return;
}

void user_to_user(struct procent *ptold, pid32 calling_pid)
{
	// print_ready_list();
	if (ptold->prstate == PR_CURR) { 
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		// kprintf("This is a system process with pid: %d and name: %s\n",currpid,ptold->prname);
		ptold->prstate = PR_READY;
		insert_user_process(currpid, user_list, ptold->tickets);
		}
		
		new_pid = Find_winner();
		if(new_pid == currpid)
		{
			return;
		}
		dequeue_user_list(new_pid,user_list);
		currpid = new_pid;
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;	
		ptnew->num_ctxsw++;
		#ifdef DEBUG_CTXSW
			if(currpid != calling_pid)
			{
				kprintf("4ctxsw::%d-%d\n",calling_pid,currpid);
			}

		#endif
		ctxsw(&ptold->prstkptr, &ptnew->prstkptr);


		return;
}
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	

	// kprintf("process entering is %d with state %d and priority %d\n",currpid,proctab[currpid].prstate,proctab[currpid].prprio);
	pid32 calling_pid = currpid;
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	ptold = &proctab[currpid];
	// print_ready_list();
	if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == FALSE)		// To get system process only
	{
		if(ptold->prstate == PR_CURR)
		{
			if(firstid(readylist) != 0)			//System
			{
				// kprintf("system current to system\n");
				system_to_other(ptold,calling_pid);
			}
			else if (nonempty(user_list))			// user
			{
				// kprintf("system current to user\n");
				system_to_user(ptold,calling_pid);
			}
			else									// NULL
			{
				// kprintf("system current to null\n");
				system_to_other(ptold,calling_pid);
			}
		}
		else
		{
			// print_queue(readylist);
			if(firstid(readylist) != 0)			//System
			{
				// kprintf("system not current to system\n");
				system_to_other(ptold,calling_pid);
			}
			else if (nonempty(user_list))			// user
			{
				// kprintf("system not current to user\n");
				// print_queue(user_list);
				system_to_user(ptold,calling_pid);
			}
			else									// NULL
			{
				// kprintf("system not current to null\n");
				system_to_other(ptold,calling_pid);
			}
		}
		
	}
	else if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == TRUE && ptold->prstate == PR_CURR)		// User process only
	{
		// kprintf("This is a user process with pid: %d and state: %d and tickets: %d\n",currpid,ptold->prstate,ptold->tickets);
		// print_ready_list();
		print_queue(readylist);
		if(firstid(readylist) != 0)			//System
		{
			// kprintf("user current to system\n");
			user_to_system(ptold,calling_pid);
		}
		else if (nonempty(user_list))			// user
		{
			// kprintf("user current to user\n");
			user_to_user(ptold,calling_pid);
		}
		else
		{
			// kprintf("user current to null\n");
			user_to_system(ptold,calling_pid);
		}
	}
	else		// For null process
	{
		// print_ready_list();
		// print_queue(readylist);
		if(firstid(readylist) != 0)
		{
			// kprintf("user not current or null any to system\n");
			system_to_other(ptold,calling_pid);
		}
		else if(nonempty(user_list))
		{
			// kprintf("user not current or null any to user\n");
			system_to_user(ptold,calling_pid);
		}
		else
		{
			// kprintf("user not current or null any to null\n");
			system_to_other(ptold,calling_pid);
		}
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