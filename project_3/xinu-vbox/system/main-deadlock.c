/*  main.c  - main */

#include <xinu.h>

#define N 5 // Note: 3*N+4 must be <= NALOCKS 

pid32 pid[3*N+5];		// threads 
al_lock_t mutex[3*N+5];		// mutexes
al_lock_t *prevention;
syscall sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}


void run_for_ms(uint32 time){
	uint32 start = proctab[currpid].runtime;
	while ((proctab[currpid].runtime-start) < time);
}

process p1(al_lock_t *l1, al_lock_t *l2){
//	sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);	
	al_lock(l1);
	run_for_ms(1000);
	al_lock(l2);		
	run_for_ms(1000);
	al_unlock(l1);
	run_for_ms(1000);
	al_unlock(l2);		
	run_for_ms(1000);
	return OK;
}


process p2(al_lock_t *l1, al_lock_t *l2){
//	sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);	
	al_lock(prevention);
	al_lock(l1);
	run_for_ms(1000);
	al_lock(l2);		
	run_for_ms(1000);
	al_unlock(l1);
	run_for_ms(1000);
	al_unlock(l2);		
	run_for_ms(1000);
	al_unlock(prevention);
	return OK;
}

process p3(al_lock_t *l1, al_lock_t *l2){
//	sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);	
	
	while (1) {
        
        al_lock(l1);
        run_for_ms(1000); 
        if (!al_trylock(l2)) 
		{
			al_unlock(l1);
        	run_for_ms(1000);
        }  
        break;
    }
    run_for_ms(1000);
    al_unlock(l2);
    al_unlock(l1);

    return OK;
}

process p4(al_lock_t *l1, al_lock_t *l2){
//	sync_printf("P%d:: acquiring: l1=%d l2=%d\n", currpid, l1->id, l2->id);	
	if(l1 > l2)
	{
		al_lock(l1);
		run_for_ms(1000);
		al_lock(l2);		
		run_for_ms(1000);

	}
	else
	{
		al_lock(l2);
		run_for_ms(1000);
		al_lock(l1);		
		run_for_ms(1000);
	}
	al_unlock(l1);
	run_for_ms(1000);
	al_unlock(l2);		
	run_for_ms(1000);

	return OK;
}

	
process	main(void)
{

	uint32 i;
	
	/* initialize al_locks */
	for (i=0; i<3*N+5; i++) al_initlock(&mutex[i]);

	kprintf("\n\n=========== TEST for deadlocks  ===================\n\n");

	pid[0] = create((void *)p1, INITSTK, 2, "part1", 2, &mutex[0], &mutex[1]);	
	pid[1] = create((void *)p1, INITSTK, 2, "part1", 2, &mutex[1], &mutex[2]);
	pid[2] = create((void *)p1, INITSTK, 2, "part1", 2, &mutex[2], &mutex[0]);


	resume(pid[0]);
	sleepms(100);
	resume(pid[1]);
	sleepms(100);
	resume(pid[2]);

	sleepms(3000);
	kprintf("\n\n=========== TEST for preventing deadlock by Avoiding Hold and wait  ===================\n\n");
	al_initlock(prevention);
	pid[3] = create((void *)p2, INITSTK, 2, "part2", 2, &mutex[3], &mutex[4]);	
	pid[4] = create((void *)p2, INITSTK, 2, "part2", 2, &mutex[4], &mutex[5]);
	pid[5] = create((void *)p2, INITSTK, 2, "part2", 2, &mutex[5], &mutex[3]);

	resume(pid[3]);
	sleepms(100);
	resume(pid[4]);
	sleepms(100);
	resume(pid[5]);

	kprintf("P%d completed\n", receive());
	

	kprintf("\n\n=========== TEST for preventing deadlock by allowing preemption  ===================\n\n");
	pid[6] = create((void *)p3, INITSTK, 2, "part3", 2, &mutex[6], &mutex[7]);	
	pid[7] = create((void *)p3, INITSTK, 2, "part3", 2, &mutex[7], &mutex[8]);
	pid[8] = create((void *)p3, INITSTK, 2, "part3", 2, &mutex[8], &mutex[6]);

	resume(pid[6]);
	sleepms(100);
	resume(pid[7]);
	sleepms(100);
	resume(pid[8]);

	kprintf("P%d completed\n", receive());


	kprintf("\n\n=========== TEST for preventing deadlock by avoiding circular wait  ===================\n\n");
	pid[9] = create((void *)p4, INITSTK, 2, "part4", 2, &mutex[9], &mutex[10]);	
	pid[10] = create((void *)p4, INITSTK, 2, "part4", 2, &mutex[10], &mutex[11]);
	pid[11] = create((void *)p4, INITSTK, 2, "part4", 2, &mutex[11], &mutex[9]);

	resume(pid[9]);
	sleepms(100);
	resume(pid[10]);
	sleepms(100);
	resume(pid[11]);

	kprintf("P%d completed\n", receive());
	return OK;
}
