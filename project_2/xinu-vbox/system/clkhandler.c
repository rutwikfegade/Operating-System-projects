/* clkhandler.c - clkhandler */

#include <xinu.h>

/*------------------------------------------------------------------------
 * clkhandler - high level clock interrupt handler
 *------------------------------------------------------------------------
 */
void	clkhandler()
{
	static	uint32	count1000 = 1000;	/* Count to 1000 ms	*/
	// extern uint32	ctr1000;
	struct	procent	*prptr;
	prptr = &proctab[currpid];
	/* Decrement the ms counter, and see if a second has passed */
	
	// if(prptr->prstate == PR_CURR && prptr->user_process == TRUE)
	// {
	// 	prptr->runtime += ctr1000-prptr->start_time;
	// 	prptr->start_time = ctr1000;
	// }
	struct procent *current_process = &proctab[currpid];

    // // Print relevant information about the current process
    // kprintf("Current Running Process:\n");
    // kprintf("PID: %d\n", currpid);
    // kprintf("Process Name: %s\n", current_process->prname);
    // kprintf("Process State: %d\n", current_process->prstate);
    // kprintf("User Process: %s\n", current_process->user_process ? "Yes" : "No");
    // kprintf("Number of Context Switches: %d\n", current_process->num_ctxsw);
	ctr1000++;
	prptr->runtime++;

	if((--count1000) <= 0) {

		/* One second has passed, so increment seconds count */

		clktime++;

		/* Reset the local ms counter for the next second */

		count1000 = 1000;
	}

	/* Handle sleeping processes if any exist */

	if(!isempty(sleepq)) {

		/* Decrement the delay for the first process on the	*/
		/*   sleep queue, and awaken if the count reaches zero	*/

		if((--queuetab[firstid(sleepq)].qkey) <= 0) {
			wakeup();
		}
	}

	/* Decrement the preemption counter, and reschedule when the */
	/*   remaining time reaches zero			     */

	if((--preempt) <= 0) {
		preempt = QUANTUM;
		resched();
	}
}
