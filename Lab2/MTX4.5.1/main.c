
#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

int procSize = sizeof(PROC);
int nproc, rflag;			// number of procs, re-schedule flag

#include "bio.c"				// may include io.c and queue.c here
#include "queue.c"
#include "wait.c"			// ksleep(), kwakeup(), kexit(), wait()
#include "kernel.c"			// other kernel functions

int init()
{
	PROC *p; int i;
	for (i=0; i<NPROC; i++) 	// initialize all procs
	{
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;
		p->next = &proc[i+1];
	}

	freeList = &proc[0]; proc[NPROC-1].next = 0; // freeList
	readyQueue = sleepList = 0;

	/****** create P0 as running ******/
	p = get_proc(&freeList); // get PROC from freeList
	p->status = READY;
	running = p;
	nproc = 1;
}

int scheduler()
{
	if (running->status == READY)
		enqueue(&readyQueue, running);

	running = dequeue(&readyQueue);
	rflag = 0;
}

main()
{

	printf("MTX starts in main()\n");
	init();					// initialize and create P0 as running
	kfork();				// P0 kfork() P1 to run body()

	while(1)
	{
		while(!readyQueue);		// P0 idle loop while readyQueue empty
		tswitch();			// P0 switch to run P1
	}
}

