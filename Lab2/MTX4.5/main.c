/********************* main.c file of MTX4.5 kernel ******************/

#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

int procSize = sizeof(PROC);
int nproc, rflag;			// number of procs, re-schedule flag

// Function Prototypes
int init();
int scheduler();
PROC *kfork();
int body();
int main();

#include "io.c"				// may include io.c and queue.c here
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
			/* Currently not supporting these functions
			case 't' : do_stop(); break;
			case 'c' : do_continue(); break;
			case 'z' : do_sleep(); break;
			case 'a' : do_wakeup(); break;
			case 'p' : do_chpriority(); break;
			*/
			case 'w' : do_wait(); break;
			default: printf("invalid command\n"); break;
		}
	}
}

main()
{
	printf("MTX starts in main()\n");

	init();					// initialize and create P0 as running
	kfork();				// P0 kfork() P1 to run body()

	while(1)
	{
		/*
		printf("P0 is running.\n");
	
		if (nproc == 2 && proc[1].status != READY)
			printf("No processes available.\n");

		while(!readyQueue);		// P0 idle loop while readyQueue empty
		printf("P0 calls tswitch().\n");	
		tswitch();			// P0 switch to run P1
		*/

		while(!readyQueue);		// P0 idle loop while readyQueue empty
		tswitch();			// P0 switch to run P1
	}
}

