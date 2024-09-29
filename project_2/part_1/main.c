/*  main.c  - main */

#include <xinu.h>

process test_process() {
   
    pid32 pid = currpid;
	volatile int i;
  
    uint32 run_time = 5000;
    uint32 elapsed_time = 0;
	uint32 sleep_time = 0;
    
    while (elapsed_time < run_time) {
        
        for (i = 0; i < 1000; i++);

       
        uint32 sleep_start = ctr1000;
        sleepms(100);
        uint32 sleep_end = ctr1000;


        sleep_time += (sleep_end - sleep_start);
        elapsed_time+=100;
    }

  
    kprintf("Process %d: Expected runtime: %d ms, Actual runtime: %d ms\n Time sleeping: %d \n", pid, elapsed_time, proctab[pid].runtime,sleep_time);
    return OK;
}

process	main(void)
{
	pid32	shpid;		/* Shell process ID */

	printf("\n\n");

	/* Create a local file system on the RAM disk */

	lfscreate(RAM0, 40, 20480);

	/* Run the Xinu shell */

	recvclr();
	resume(shpid = create(shell, 8192, 50, "shell", 1, CONSOLE));
	pid32 test_pid = create_user_process(test_process, 1024, "TestProcess", 0);
	resume(test_pid);
	/* Wait for shell to exit and recreate it */

	while (TRUE) {
	    if (receive() == shpid) {
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(shpid = create(shell, 4096, 20, "shell", 1, CONSOLE));
	    }
	}
	return OK;
}
