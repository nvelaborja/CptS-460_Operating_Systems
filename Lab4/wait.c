/******************* wait.c file ****************************/

int ksleep(int event) 
{ 
	printf("P%d Enter ksleep \n", running->pid);
	running->event = event;		// record event in PROC.event
	running->status = SLEEP;	// change status to SLEEP
	enqueue(&sleepList, running);
	tswitch();					// give up CPU
}

int kwakeup(int event)
{ 
	int i; PROC *p;

printf("P%d Enter kwakeup \n", running->pid);

	for (i = 1; i < NPROC; i++)		// skip P0
	{
		p = &proc[i];
		if (p->status == SLEEP && p->event == event)
		{
			//dequeue(&sleepList, p);		// Remove from sleep list
			p->event = 0;				// cancel PROC's event
			p->status = READY;			// make it ready to run again
			enqueue(&readyQueue, p);	// Put in ready queue
		}
	}
}

int ready(PROC *p) 
{ 
	printf("P%d Enter ready \n", running->pid);

	p->event = 0;
	p->status=READY; 
	enqueue(&readyQueue, p); 
	printf("Wakeup P%d\n", p->pid);
}

int kexit(int exitValue)
{
	PROC *p = 0;
	int i, wakeupP1 = 0;

	printf("P%d Enter kexit \n", running->pid);

	if (running->pid==1 && nproc>2)				// nproc = number of active PROCs
	{ 
		printf("other procs still exist, P1 can't die yet\n");
		return -1;
	}

	/* send children (dead or alive) to P1's orphanage */
	for (i = 1; i < NPROC; i++)
	{
		p = &proc[i];

		if (p->status != FREE && p->ppid == running->pid)
		{
			p->ppid = 1;
			p->parent = &proc[1];
			wakeupP1++;
		}

	}

	/* record exitValue and become a ZOMBIE */
	running->exitCode = exitValue;
	running->status = ZOMBIE;

	/* wakeup parent and also P1 if necessary */
	kwakeup(running->parent); 				// parent sleeps on its PROC address

	if (wakeupP1)
		kwakeup(&proc[1]);

	tswitch();						// give up CPU

	return 0;
}

int kwait(int *status) 							// wait for ZOMBIE child
{
	PROC *p; int i, hasChild = 0;

	printf("P%d Enter kwait \n", running->pid);

	while(1)
	{
 		// search PROCs for a child
		for (i=1; i<NPROC; i++)					// exclude P0
		{
			p = &proc[i];
		
			printf("ppid: %d | running->pid %d | p->pid %d\n", p->ppid, running->pid, p->pid);

			if (p->status != FREE && p->ppid == running->pid)
			{
				hasChild = 1;					// has child flag
				
				if (p->status == ZOMBIE)
				{
		 										// lay the dead child to rest
					*status = p->exitCode;		// collect its exitCode
					p->status = FREE;			// free its PROC
					p->priority = 0;			// reset priority
					put_proc(&freeList, p); 	// to freeList
					nproc--;					// once less processes
			
					return(p->pid);				// return its pid
				}
			}
		}

		if (!hasChild) return -1;				// no child, return ERROR
		
		ksleep(running); 						// still has kids alive: sleep on PROC address
	}

	return 0;
}



