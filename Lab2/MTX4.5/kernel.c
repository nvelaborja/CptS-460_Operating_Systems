/********************** kernel.c file ***********************/

// Function Prototypes
int do_tswitch();
int do_kfork();
int do_exit();
int do_wait();
int do_stop();
int do_continue();
int do_sleep();
int do_wakeup();
int reschedule();
int chpriority(int pid, int pri);
int do_chpriority();
int atoi(char *s);
int geti();

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

	if (running->pid == 1 && nproc > 2)		// If P1 and there are other procs running than P1 and P0
	{
		printf("P1 can't die yet.\n");
		return -1;				// Don't exit
	}

	// Otherwise
	printf("Enter an exit value (0 - 9): ");	// Prompt for exit value
	exitValue = (getc()&0x7F) - '0';
	
	kexit(exitValue);
}

int do_wait()
{
	int child, status;
	
	child = kwait(&status);

	if (child < 0)
	{
		printf("P%d wait error : no child\n", running->pid);
		return -1;
	}
		
	printf("P%d found a zombie child %d with exit value %d\n", running->pid, child, status);

	return child;	
}

int do_stop()		// Not supported in MTX 4.5
{ 

}

int do_continue()	// Not supported in MTX4.5
{ 

}

int do_sleep()		// Not supported in MTX4.5
{ 
	ksleep();
}

int do_wakeup() 	// Not supported in MTX4.5
{ 
	kwakeup();
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

PROC *kfork()  // create a child process, begin from body()
{
	int i;
	PROC *p = get_proc(&freeList);
	if (!p) {
		printf("no more PROC, kfork() failed\n");
		return 0;
	}
	p->status = READY;
	p->priority = 1;				// priority = 1 for all proc except P0
	p->ppid = running->pid;				// parent = running
	
	// initialize new proc's kstack[]  
	for (i = 1; i < 10; i++)			// saved CPU registers
		p->kstack[SSIZE-i]= 0;			// all 0's

	// for ds and ss
	p->kstack[SSIZE - 10] = 0x1000;
	p->kstack[SSIZE - 11] = 0x1000;

	p->kstack[SSIZE-1] = (int)body;			// resume point=address of body()
	//p->ksp = &p->kstack[SSIZE-9];			// proc saved sp
	p->ksp = &p->kstack[SSIZE-11];			// proc saved sp			
	enqueue(&readyQueue, p);			// enter p into readyQueue by priority
	return p;					// return child PROC pointer
}s

int chpriority(int pid, int pri)	// Not supported in MTX4.5
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
