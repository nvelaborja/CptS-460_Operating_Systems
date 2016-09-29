/********************** kernel.c file ***********************/

int rflag, body();

void PrintProccess(PROC *p);

int body()
{
	char c;

	///printf("P%d Enter body \n", running->pid);

	while(1)
	{
		color = running->pid + 7;
		if (rflag)
		{
			printf("P%d: reschedule\n", running->pid);
			rflag = 0;
			tswitch();
		}
		printf("-------------------------------------------------------------\n");
		printList("freelist ", freeList);			// show freelist
		printQueue("readyQueue", readyQueue);		// show readyQueue
		printf("-------------------------------------------------------------\n");

		printf("P%d running: priority=%d parent=%d enter a char [ s | f | w | q | u ]: ", running->pid, running->priority, running->parent->pid );

		c = getc(); printf("%c\n", c);

		switch(c)
		{
			case 's' : do_tswitch(); break;
			case 'f' : do_kfork(); break;
			case 'q' : do_exit(); break;
			/* Currently not supporting these functions
			case 't' : do_stop(); break;
			case 'c' : do_continue(); break;
			case 'z' : do_sleep(); break;
			case 'a' : do_wakeup(); break;
			case 'p' : do_chpriority(); break;
			*/
			case 'w' : do_wait(); break;
			case 'u' : goUmode(); break;
			default: printf("Invalid Command.\n"); break;
		}
	}
}

int atoi(char *s)
{
  int v = 0;

//printf("P%d Enter atio \n", running->pid);

  while(*s){
    v = v*10 + (*s-'0');
    s++;
  }
  return v;
}

int geti()
{
  char s[16];

  printf("P%d Enter geti \n", running->pid);

  gets(s);
  return atoi(s);
}

PROC *kfork(char *filename) 				// create a child process, begin from body()
{
	PROC *p = get_proc(&freeList);
	int i, child;
	u16 segment;

	//printf("P%d Enter kfork with filename '%s'\n", running->pid, filename);

	if (!p)
	{
		printf("no more PROC, kfork() failed\n");
		return -1;
	}

	p->status = READY;
	p->priority = 1; 						// priority == 1 for all procs except P0
	p->ppid = running->pid; 				// parent == running
	p->parent = running;					// parent == current running proc

	for (i=1; i<10; i++)					// Initialize new proc's kstack
		p->kstack[SSIZE-i] = 0;

	p->kstack[SSIZE-1] = (int)goUmode; 		// resume point=address of goUmode
	p->ksp = &(p->kstack[SSIZE-9]);			// proc saved stack pointer

	enqueue(&readyQueue, p); 				// enter p in readyqueue by priority
	nproc++;

	PrintProccess(p);

	segment = IMGSIZE * (p->pid + 1);	// Hard code segment location for now

	// Now create image if filename was specified
	if (filename)							
	{
		load(filename, segment);			// Load file into segment

		for (i = 1; i < 12; i++)			// Initialize new image
			put_word(0, segment, -2 * i);

		p->usp = -2 * 12;					// usp is relative to uss
		p->uss = segment;

		put_word(0x0200, segment, -2 * 1);		// flag
		put_word(segment, segment, -2 * 2);		// uCS
		put_word(segment, segment, -2 * 11);	// uES
		put_word(segment, segment, -2 * 12);	// uDS
	}

	printf("kfork: P%d forked P%d at %x\n", running->pid, p->pid, segment);

	return p; 								// return child PROC pointer
}

void PrintProccess(PROC *p)
{
	printf("\n----- P%d -----\n", p->pid);
	printf("name: %s\n", p->name);
	printf("pid: %d\n", p->pid);
	printf("ppid: %d\n", p->ppid);
	printf("status: %d\n", p->status);
	printf("priority: %d\n", p->priority);
	printf("-----------------\n\n");
}

int do_tswitch() 
{ 
	//printf("P%d Enter do_tswitch \n", running->pid);
	tswitch();
}

int do_kfork()
{ 
	PROC *p = kfork("/bin/u1");

	if (p == 0)
	{
		printf("kfork failed.\n");
		return -1;
	}

	printf("PROC %d kfork a child %d\n", running->pid, p->pid);
	return p->pid;
}

int do_exit()
{ 
	int i = 0;
	char c;

	//printf("P%d Enter do_exit \n", running->pid);

	if (running->pid == 1 && nproc > 2)
	{
		printf("P1 can't die, too many children to take care of.\n");
		return -1;
	}

	printf("Enter exit value (0 - 9): ");

	c = getc(); printf("%c\n", c);
	i = c - '0';

	kexit(i);
}

int do_wait()
{
  int child, status;
  child = kwait(&status);

//printf("P%d Enter do_wait \n", running->pid);

  if (child<0){
    printf("P%d wait error : no child.\n", running->pid);
    return -1;
  }
  printf("P%d found a ZOMBIE child, P%d with exitValue %d\n", 
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

	//printf("P%d Enter reschedule \n", running->pid);

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

	//printf("P%d Enter chpriority \n", running->pid);
	
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

//printf("P%d Enter do_chpriority \n", running->pid);

	printf("input pid " );
	pid = geti();

	printf("input new priority " );
	pri = geti();

	if (pri<1) pri = 1;

	chpriority(pid, pri);
}













