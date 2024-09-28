#include <xinu.h>

 

pid32 fork()
{
	uint32		savsp, *pushsp;
	intmask 	mask;    	
	pid32		pid;		
	struct	procent	*prptr;		
    struct	procent	*parent_pointer;		
	uint32		*saddr;		
    uint32      ssize;
    pri16       priority;
    uint32      *parent_base;
    uint32      *parent_sp;
    unsigned long	*sp, *fp;

    parent_pointer = &proctab[currpid];
    ssize = parent_pointer->prstklen;
    priority = parent_pointer->prprio;
    parent_base = parent_pointer->prstkbase;
    parent_sp = parent_pointer->prstkptr;

	#ifdef DEBUG
    	kprintf("base is %X and sp is %X\n",parent_base,parent_sp);
	#endif

	mask = disable();
	if (ssize < MINSTK)
		ssize = MINSTK;
	ssize = (uint32) roundmb(ssize);
	if ( (priority < 1) || ((pid=newpid()) == SYSERR) ||
	     ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR) ) {
		restore(mask);
		return SYSERR;
	}
	
	#ifdef DEBUG
    	kprintf("child process address %X\n",saddr);
	#endif
		asm("movl %%esp, %0\n" :"=r"(sp));
		asm("movl %%ebp, %0\n" :"=r"(fp));
	#ifdef DEBUG
    	kprintf("sp = %X and fp = %X\n\n",sp,fp);
	#endif
	
	prcount++;
	prptr = &proctab[pid];
	/* Initialize process table entry for new process */
	prptr->prstate = PR_SUSP;	/* Initial state is suspended	*/
	prptr->prprio = priority;
	prptr->prstkbase = (char *)saddr;
	prptr->prstklen = ssize;
	prptr->prname[PNMLEN-1] = NULLCH;
	prptr->user_process = FALSE; 
	// for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=name[i])!=NULLCH; i++)
	// 	;
	prptr->prsem = -1;
	prptr->prparent = (pid32)getpid();
	prptr->prhasmsg = FALSE;

	/* Set up stdin, stdout, and stderr descriptors for the shell	*/
	prptr->prdesc[0] = CONSOLE;
	prptr->prdesc[1] = CONSOLE;
	prptr->prdesc[2] = CONSOLE;

	uint32 *child_fp = saddr;  
    uint32 *parent_fp = fp; 
	*saddr = STACKMAGIC;
	savsp = (uint32)saddr;
	uint32 frame_difference = (uint32)parent_base - (uint32)saddr;
	while (parent_base > parent_fp) 
	{
		*(--saddr) = *(--parent_base); 
		#ifdef DEBUG
			kprintf("DATA (%X) at %X, base %X\n", saddr, *saddr, parent_base);
		#endif
	}
	#ifdef DEBUG	
		child_fp = (uint32 *)((uint32)parent_fp - frame_difference);
		kprintf("child_fp = %X\n",child_fp);
		
		uint32 *current_fp = child_fp;
		*current_fp = *current_fp - frame_difference;
		current_fp = (uint32 *)(*current_fp);
		kprintf("*curr_fp %X and curr_fp %X\n",*current_fp,current_fp);
		*current_fp = *current_fp - frame_difference;
		current_fp = (uint32 *)(*current_fp);
		kprintf("*curr_fp %X and curr_fp %X\n",*current_fp,current_fp);
	#endif
	uint32 *current_fp = parent_fp;
    uint32 *child_fp_address;

	/*To Remember:- The way this logic works is it takes the top address of the parent stack process and subtracts is with top address 
	  of the child stack process, this gives us the size difference between parent and child. Now we subtract that difference with 
	  the parent top address giving us the top address of the child stack, as we know that stak process stores fp at the top of the 
	  stack, now similarly i keep on decreading the difference and getting the other fp and assigning them the fp of next frame*/ 

   	while (current_fp != NULL) 
	{ 
        child_fp_address = (uint32 *)((uint32)current_fp - frame_difference);
        uint32 temp_fp = *current_fp;

        if (temp_fp >= (uint32)parent_sp && temp_fp <= (uint32)parent_pointer->prstkbase) 
		{
            *child_fp_address = temp_fp - frame_difference;
			#ifdef DEBUG
            	kprintf("Frame pointer at child stack location %X to new value %X\n", child_fp_address, *child_fp_address);
			#endif
        } 
		else 
		{
			break;
		}

		current_fp = (uint32 *)temp_fp;
		#ifdef DEBUG
			kprintf("Current parent fp: %X, New child fp: %X\n", temp_fp, *child_fp_address);
		#endif
    }
	
	/*To Remeber:- we add the registers after the copying of the parent stack, this creates a new frame for them*/

	savsp = (uint32) saddr;		
	*--saddr = 0x00000200;		
	*--saddr = NPROC;			// As we cant return 0 in xinu for a process (system process) we return nproc		
	*--saddr = 0;			
	*--saddr = 0;			
	*--saddr = 0;			
	*--saddr = 0;			
	pushsp = saddr;			
	*--saddr = (uint32)child_fp;		//Assigning the new fp to child
	*--saddr = 0;			
	*--saddr = 0;			
	*pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);

	ready(pid);

	#ifdef DEBUG
		stacktrace(pid);
	#endif

	restore(mask);

	return pid;
}


