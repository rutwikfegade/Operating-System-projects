/* resched.c - resched, resched_cntl */

#include <xinu.h>
// #define DEBUG


struct	defer	Defer;
struct procent *ptnew;	/* Ptr to table entry for new process	*/
extern uint32 total_process;
/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void Time_allotment_expired()
{
	struct procent *process;
	process = &proctab[currpid];

	if(process->prprio > 1)
	{
		process->prprio -= 1;
		// kprintf("Process %d is on Queues number %d\n",currpid,process->prprio);
		process->downgrades += 1;
		process->time_allotment = TIME_ALLOTMENT * (1U << (UPRIORITY_QUEUES - process->prprio));
	}
	// else
	// {
	// 	process->time_allotment = TIME_ALLOTMENT * (1U << (UPRIORITY_QUEUES - process->prprio));
	// }
	
}

void priority_boost()
{
	boost_period = 0;
	int i;
	for(i = 0; i<NPROC; i++)
	{
		if(proctab[i].user_process == TRUE && proctab[i].prstate != PR_FREE)
		{
			// kprintf("boosting %d\n",i);
			proctab[i].upgrades += 1;
			proctab[i].prprio = UPRIORITY_QUEUES;
			proctab[i].time_allotment = TIME_ALLOTMENT;
			if(proctab[i].prstate == PR_READY)
			{
				getitem(i);
				insert(i,user_list,proctab[i].prprio);
			}
		}
	}
}



void system_to_other(struct procent *ptold, pid32 calling_pid)
{
	// print_ready_list();
	// print_queue(readylist);
	// kprintf("This is a system to other process with pid: %d and state: %d\n",currpid,ptold->prstate);
	if (ptold->prstate == PR_CURR) { 
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
		}
		currpid = dequeue(readylist);
		// kprintf("currpid %d\n",currpid);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;	
		if(currpid != calling_pid)
		{
			// if(currpid == 5 || calling_pid == 5)
			// 	kprintf("context switch in system to other from %d to %d\n",calling_pid,currpid);
			ptnew->num_ctxsw++;
		}
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
	// kprintf("This is a user to system process with pid: %d and state: %d\n",currpid,ptold->prstate);
	if (ptold->prstate == PR_CURR) { 
	ptold->prstate = PR_READY;
	insert(currpid, user_list, ptold->prprio);
	}
	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;	
	if(currpid != calling_pid)
	{
		// if(currpid == 5 || calling_pid == 5)
		// 	kprintf("context switch in user to system from %d to %d\n",calling_pid,currpid);
		ptnew->num_ctxsw++;
	}
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
	// kprintf("Entering process %d with state %d\n",currpid,ptold->prstate);
	// kprintf("This is a system to user process with pid: %d and state: %d\n",currpid,ptold->prstate);
	if (ptold->prstate == PR_CURR) { 
		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
		}

		currpid = dequeue(user_list);
		ptnew = &proctab[currpid];
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;	
		if(currpid != calling_pid)
		{
			// if(currpid == 5 || calling_pid == 5)
			// 	kprintf("context switch in system to user from %d to %d\n",calling_pid,currpid);
			ptnew->num_ctxsw++;
		}
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
	// kprintf("Total process are %d and is the queue empty %d\n",total_process,isempty(user_list));
	// if(total_process == 1 && isempty(user_list))
	// {
	// 	return;
	// }
	pid32 Entering_pid = currpid;
	// kprintf("This is a user to user process with pid: %d and state: %d\n",currpid,ptold->prstate);
	if (ptold->prstate == PR_CURR) { 
		ptold->prstate = PR_READY;
		insert(currpid, user_list, ptold->prprio);
		}
		currpid = dequeue(user_list);
		ptnew = &proctab[currpid];
		if(currpid != calling_pid)
		{
			// if(currpid == 5 || calling_pid == 5)
			// 	kprintf("context switch in user to user from %d to %d\n",calling_pid,currpid);
			ptnew->num_ctxsw++;
		}
		ptnew->prstate = PR_CURR;
		preempt = QUANTUM;	
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
	// kprintf("process calling the resched is %d with state %d and ctxsw value of %d\n",currpid,proctab[currpid].prstate,proctab[currpid].num_ctxsw);
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	pid32 calling_pid = currpid;
	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		// kprintf("Defered\n");
		Defer.attempt = TRUE;
		return;
	}

	if(boost_period >= PRIORITY_BOOST_PERIOD)
	{
		// kprintf("boosting\n");
		priority_boost();
	}

	ptold = &proctab[currpid];

	if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == FALSE)		// To get system process only
	{
		if(ptold->prstate == PR_CURR)
		{
			if(firstid(readylist) != 0)			//System
			{
				#ifdef DEBUG
					kprintf("system current to system, entered pid %d\n",currpid);
				#endif
				system_to_other(ptold,calling_pid);
			}
			else if (nonempty(user_list))			// user
			{
				#ifdef DEBUG
					kprintf("system current to user, entered pid %d\n",currpid);
				#endif
				// if(ptold->time_allotment <= 0)
				// {
				// 	Time_allotment_expired();
				// }
				system_to_user(ptold,calling_pid);
			}
			else									// NULL
			{
				#ifdef DEBUG
					kprintf("system current to null, entered pid %d\n",currpid);
				#endif
				system_to_other(ptold,calling_pid);
			}
		}
		else
		{
			// print_queue(readylist);
			if(firstid(readylist) != 0)			//System
			{
				#ifdef DEBUG
					kprintf("system not current to system, entered pid %d\n",currpid);
				#endif
				system_to_other(ptold,calling_pid);
			}
			else if (nonempty(user_list))			// user
			{
				#ifdef DEBUG
					kprintf("system not current to user, entered pid %d\n",currpid);
				#endif
				// if(ptold->time_allotment <= 0)
				// {
				// 	Time_allotment_expired();
				// }
				// print_queue(user_list);
				system_to_user(ptold,calling_pid);
			}
			else									// NULL
			{
				#ifdef DEBUG
					kprintf("system not current to null, entered pid %d\n",currpid);
				#endif
				system_to_other(ptold,calling_pid);
			}
		}
		
	}
	else if(strcmp(ptold->prname, "prnull") != 0 && ptold->user_process == TRUE && ptold->prstate == PR_CURR)		// User process only
	{
		// kprintf("This is a user process with pid: %d and state: %d and tickets: %d\n",currpid,ptold->prstate,ptold->tickets);
		// print_ready_list();
		// print_queue(readylist);
		// print_queue(user_list);
		// kprintf("total number of process available %d\n",total_process);
		if(firstid(readylist) != 0)			//System
		{
			#ifdef DEBUG
				kprintf("user current to system, entered pid %d\n",currpid);
			#endif
			user_to_system(ptold,calling_pid);
		}
		else if (nonempty(user_list) || ptold->prstate == PR_CURR)			// user
		{
			#ifdef DEBUG
				kprintf("user current to user, entered pid %d\n",currpid);
			#endif
			if(ptold->time_allotment <= 0)
			{
				Time_allotment_expired();
			}
			user_to_user(ptold,calling_pid);
		}
		else
		{
			#ifdef DEBUG
				kprintf("user current to null, entered pid %d\n",currpid);
			#endif
			// kprintf("user current to null, entered pid %d and total_ process %d\n",currpid,total_process);
			if(ptold->time_allotment <= 0)
			{
				Time_allotment_expired();
			}
			user_to_system(ptold,calling_pid);
		}
	}
	else		// For null process
	{
		// print_ready_list();
		// print_queue(readylist);
		if(nonempty(readylist) && firstid(readylist) != 0)
		{
			#ifdef DEBUG
				kprintf("user not current or null any to system, entered pid %d\n",currpid);
			#endif
			if(ptold->user_process == TRUE)
			{
				user_to_system(ptold,calling_pid);
			}
			else
			{
				system_to_other(ptold,calling_pid);
			}
			
		}
		else if(nonempty(user_list))
		{
			#ifdef DEBUG
				kprintf("user not current or null any to user, entered pid %d\n",currpid);
			#endif
			if(ptold->time_allotment <= 0)
			{
				Time_allotment_expired();
			}
			system_to_user(ptold,calling_pid);
		}
		else
		{
			#ifdef DEBUG
				kprintf("user not current or null any to null, entered pid %d\n",currpid);
			#endif
			if(ptold->user_process == TRUE)
			{
				// kprintf("inside is pid %d with state %d and tme allotment as %d\n",currpid,ptold->prstate,ptold->time_allotment);
				if(ptold->time_allotment <= 0)
				{
					Time_allotment_expired();
				}
				user_to_system(ptold,calling_pid);
			}
			else
			{
				system_to_other(ptold,calling_pid);
			}
			
		}
	}



	/* Old process returns here when resumed */

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
