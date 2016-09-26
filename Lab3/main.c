/********************* main.c file of MTX4.5 kernel ******************/

#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;				// number of procs, re-schedule flag
char *pname[] = { "Sun", "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", 0 };
extern int color;
int body();

#include "io.c"
#include "queue.c"
#include "wait.c"			// ksleep(), kwakeup(), kexit(), wait()
#include "kernel.c"			// other kernel functions
#include "int.c"			// User Mode stuff

int init()
{
	PROC *p; int i;

	printf("Initializing.");

	for (i=0; i<NPROC; i++) 	// initialize all procs
	{
		printf(".");
		p = &proc[i];
		p->pid = i;
		p->status = FREE;
		p->priority = 0;
		p->next = &proc[i+1];
		strcpy(proc[i].name, pname[i]);
	}

	freeList = &proc[0]; 
	proc[NPROC-1].next = 0; // freeList
	readyQueue = sleepList = 0;

	/****** create P0 as running ******/
	p = get_proc(&freeList); // get PROC from freeList
	p->status = RUNNING;
	p->ppid = 0;
	p->parent = p;
	running = p;
	nproc++;
	printf(" Done.\n");
}

int scheduler()
{
	printf("P%d Enter scheduler \n", running->pid);

	if (running->status == READY)
		enqueue(&readyQueue, running);
	
	running = dequeue(&readyQueue);
	running->status = RUNNING;
	rflag = 0;
}

int int80h();

int setVector(u16 vector, u16 handler)
{
	put_word(handler, 0, vector<<2);
	put_word(0x1000, 0, (vector<<2) + 2);

	return 1;
}

main()
{
	printf("MTX starts in main()\n");
	init();					// initialize and create P0 as running

	setVector(80, int80h);	// Set vector 80 so we can run uMode

	kfork("/bin/u1");		// P0 kfork() P1 to run uMode

	while(1)
	{
		if (nproc == 2 && proc[1].status != READY)
			printf("No process available, system must halt.\n");

		while(!readyQueue);	// P0 idle loop while readyQueue empty
		tswitch();			// P0 switch to run P1
	}
}