/* insert.c - insert */

#include <xinu.h>
extern int32 Total_Tickets;
/*------------------------------------------------------------------------
 *  insert  -  Insert a process into a queue in descending key order
 *------------------------------------------------------------------------
 */
status	insert(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q,		/* ID of queue to use		*/
	  int32		key		/* Key for the inserted process	*/
	)
{
	qid16	curr;			/* Runs through items in a queue*/
	qid16	prev;			/* Holds previous node index	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	curr = firstid(q);
	while (queuetab[curr].qkey >= key) {
		curr = queuetab[curr].qnext;
	}
	// kprintf("current process is : %d\n",curr);
	/* Insert process between curr node and previous node */
	// kprintf("process is: %d with key: %d\n",pid,key);
	// print_ready_list();
	prev = queuetab[curr].qprev;	/* Get index of previous node	*/
	queuetab[pid].qnext = curr;
	queuetab[pid].qprev = prev;
	queuetab[pid].qkey = key;
	queuetab[prev].qnext = pid;
	queuetab[curr].qprev = pid;
	// print_ready_list();
	return OK;
}

status	insert_user_process(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q,		/* ID of queue to use		*/
	  int32		key		/* Key for the inserted process	*/
	)
{
	qid16	curr;			/* Runs through items in a queue*/
	qid16	prev;			/* Holds previous node index	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	curr = firstid(q);
	while (queuetab[curr].qkey > key) {
		curr = queuetab[curr].qnext;
	}
	if(queuetab[curr].qkey == key)
	{
		while(queuetab[curr].qkey == key && proctab[curr].pid<pid)
		{
			curr = queuetab[curr].qnext;
		}
	}
	// Total_Tickets += proctab[curr].tickets;
	// kprintf("current process is : %d\n",curr);
	/* Insert process between curr node and previous node */
	// kprintf("process is: %d with key: %d\n",pid,key);
	// print_ready_list();
	prev = queuetab[curr].qprev;	/* Get index of previous node	*/
	queuetab[pid].qnext = curr;
	queuetab[pid].qprev = prev;
	queuetab[pid].qkey = key;
	queuetab[prev].qnext = pid;
	queuetab[curr].qprev = pid;
	// print_ready_list();
	return OK;
}
