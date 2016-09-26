/********************** kernel.c file ***********************/

int rflag, body();

int atoi(char *s)
{
  int v = 0;
  while(*s){
    v = v*10 + (*s-'0');
    s++;
  }
  return v;
}

int geti()
{
  char s[16];
  gets(s);
  return atoi(s);
}

PROC *kfork() // create a child process, begin from body()
{
	int i;
    PROC *p = get_proc(&freeList);
    if (!p)
	{
		printf("no more PROC, kfork() failed\n"); 
		return 0;
	} 
	p->status = READY;
	p->priority = 1; 
	/* initialize new proc's kstack[ ] */

	/*

	| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
	 ax  bx  cx  dx  bp  si  di   f  ds  ss

								      ^
								      add here
	*/

    

	// account for ds and ss i=1 through 11
	for (i=1; i<10; i++)
	{
    	p->kstack[SSIZE-i]= 0;
	}

	// set ds and ss to 0x1000
	p->kstack[SSIZE-10] = 0x1000;
	p->kstack[SSIZE-11] = 0x1000;
	p->kstack[SSIZE-1] = (int)body;

	// 9 + 2 = 11, for ds and ss 
	p->ksp = &p->kstack[SSIZE-11];

	/*
	(1). Complete the RESUME part, using pseudo-assembly code.
	(2). With this tswitch(), show how to initialize the kstack of a new process for it to begin execution in body().
	(3). Justify whether it is really necessary to implement context switching this way?
	(5). In MTX, tswitch() calls scheduler(), which essentially picks a runnable process from readyQueue as the next running process. 
		 Rewrite tswitch() as tswitch(PROC *current, PROC *next), which switches from the current running process to the next running process.
	(6.) In MTX, there is only one readyQueue, which contains all the processes that are READY to run. A multi-level priority queue (MPQ) consists of, e.g. n + 1 priority queues, each at a specific priority level, as shown in the following figure.
	*/


	enqueue(&readyQueue, p); // enter p into readyQueue by priority
	return p;
}

int do_tswitch() 
{ 
	tswitch();
}

int do_kfork()
{ 
	PROC *p = kfork();
	if (p == 0)
	{
		printf("kfork failed\n");
		return -1;
	}
	printf("PROC %d kfork a child %d\n", running->pid, p->pid);
	return p->pid;
}

int do_exit()
{ 
	int exitValue;

	printf("Enter an exit code (0-9): ");
	exitValue = geti();
		
	kexit(exitValue);
}

int do_wait()
{
  int child, status;
  child = kwait(&status);
  if (child<0){
    printf("proc %d wait error : no child\n", running->pid);
    return -1;
  }
  printf("proc %d found a ZOMBIE child %d exitValue=%d\n", 
	   running->pid, child, status);
  return child;
}

int do_stop()
{ 

}

int do_continue()
{ 
}

int do_sleep()
{ 

}

int do_wakeup() 
{ 

}

// added scheduling functions in MTX4.5
int reschedule()
{
	PROC *p, *tempQ = 0;

	while ( (p=dequeue(&readyQueue)) )
	{ // reorder readyQueue
		enqueue(&tempQ, p);
	}
	
	readyQueue = tempQ;
	rflag = 0;
 
	// global reschedule flag
	if (running->priority < readyQueue->priority)
		rflag = 1;
}

int chpriority(int pid, int pri)
{
	PROC *p; int i, ok = 0, reQ = 0;
	
	if (pid == running->pid)
	{
		running->priority = pri;

		if (pri < readyQueue->priority)
			rflag = 1;
		return 1;
	}

	// if not for running, for both READY and SLEEP procs
	for (i=1; i<NPROC; i++)
	{
		p = &proc[i];
		
		if (p->pid == pid && p->status != FREE)
		{
			p->priority = pri;
			ok = 1;
			
			if (p->status == READY) // in readyQueue==> redo readyQueue
				reQ = 1;
		}
	}

	if (!ok)
	{
		printf("chpriority failed\n");
		return -1;
	}

	if (reQ)
	reschedule(p);
}

int do_chpriority()
{
	int pid, pri;

	printf("input pid " );
	pid = geti();

	printf("input new priority " );
	pri = geti();

	if (pri<1) pri = 1;

	chpriority(pid, pri);
}

int body()
{
	char c;

	while(1)
	{
		if (rflag)
		{
			printf("proc %d: reschedule\n", running->pid);
			rflag = 0;
			tswitch();
		}

		printList("freelist ", freeList);		// show freelist
		printQueue("readyQueue", readyQueue);		// show readyQueue

		printf("proc%d running: priority=%d parent=%d enter a char [s|f|w|q]: ", running->pid, running->priority, running->parent->pid );

		c = getc(); printf("%c\n", c);

		switch(c)
		{
			case 's' : do_tswitch(); break;
			case 'f' : do_kfork(); break;
			case 'q' : do_exit(); break;
			case 'w' : do_wait(); break;
			default: printf("invalid command\n"); break;
		}
	}
}

