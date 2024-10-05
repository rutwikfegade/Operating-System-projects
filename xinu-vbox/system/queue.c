/* queue.c - enqueue, dequeue */

#include <xinu.h>

struct qentry	queuetab[NQENT];	/* Table of process queues	*/

/*------------------------------------------------------------------------
 *  enqueue  -  Insert a process at the tail of a queue
 *------------------------------------------------------------------------
 */
pid32	enqueue(
	  pid32		pid,		/* ID of process to insert	*/
	  qid16		q		/* ID of queue to use		*/
	)
{
	qid16	tail, prev;		/* Tail & previous node indexes	*/

	if (isbadqid(q) || isbadpid(pid)) {
		return SYSERR;
	}

	tail = queuetail(q);
	prev = queuetab[tail].qprev;

	queuetab[pid].qnext  = tail;	/* Insert just before tail node	*/
	queuetab[pid].qprev  = prev;
	queuetab[prev].qnext = pid;
	queuetab[tail].qprev = pid;
	return pid;
}

/*------------------------------------------------------------------------
 *  dequeue  -  Remove and return the first process on a list
 *------------------------------------------------------------------------
 */
pid32	dequeue(
	  qid16		q		/* ID of queue to use		*/
	)
{
	pid32	pid;			/* ID of process removed	*/

	if (isbadqid(q)) {
		return SYSERR;
	} else if (isempty(q)) {
		return EMPTY;
	}

	pid = getfirst(q);
	queuetab[pid].qprev = EMPTY;
	queuetab[pid].qnext = EMPTY;
	return pid;
}

void dequeue_user_list(pid32 pid, qid16 q)
{
	pid32	head_of_user_list;			/* ID of process removed	*/

	if (isbadqid(q)) {
		return SYSERR;
	} else if (isempty(q)) {
		return EMPTY;
	}
	head_of_user_list = firstid(q);
	// kprintf("head before: %d\n",head_of_user_list);
	queuetab[queuetab[pid].qprev].qnext = queuetab[pid].qnext;
	queuetab[queuetab[pid].qnext].qprev = queuetab[pid].qprev;
	head_of_user_list = firstid(q);
	// kprintf("head after: %d\n",head_of_user_list);
	return;
	
}

void print_queue(qid16 q) {
    if (isbadqid(q)) {
        kprintf("Invalid queue ID.\n");
        return;
    }

    qid16 curr = firstid(q);  // Get the first process in the queue

    if (isempty(q)) {
        kprintf("Queue is empty.\n");
        return;
    }

    kprintf("Queue elements (PID, key):\n");

    // Traverse the queue until we reach the end
    while (curr != queuetail(q)) {
        kprintf("PID: %d, Key: %d\n", curr, queuetab[curr].qkey);
        curr = queuetab[curr].qnext;  // Move to the next element in the queue
    }
}
