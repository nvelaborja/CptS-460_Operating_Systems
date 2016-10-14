#include "type.h"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC), rflag;
int nproc = 0;
int color;

int body();
char *pname[]={"Sun", "Mercury", "Venus", "Earth",  "Mars", "Jupiter",
               "Saturn", "Uranus", "Neptune" };

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
#include "queue.c"
#include "wait.c"             // YOUR wait.c   file
#include "kernel.c"           // YOUR kernel.c file
#include "int.c"              // YOUR int.c    file
//include "loader.c"


int init()
{
  PROC *p; int i;
  color = 0x0C;
  printf("Initializing.");
  for (i=0; i<NPROC; i++){   // initialize all procs
    putc('.');
    p = &proc[i];
    p->pid = i;
    p->status = FREE;
    p->priority = 0;
    strcpy(proc[i].name, pname[i]);
    p->next = &proc[i+1];
  }

  freeList = &proc[0];      // all procs are in freeList
  proc[NPROC-1].next = 0;
  readyQueue = sleepList = 0;

  /**** create P0 as running ******/
  p = get_proc(&freeList);
  p->status = READY;
  p->ppid   = 0;
  p->parent = p;
  running = p;
  nproc = 1;
  printf(" Done.\n");
}

int scheduler()
{
  if (running->status == READY)
    enqueue(&readyQueue, running);
  running = dequeue(&readyQueue);
  color = running->pid + 0x0A;
}

int int80h();
int set_vector(u16 vector, u16 handler)
{
  // put_word(word, segment, offset)
  put_word(handler, 0, vector<<2);             // KPC points to handler
  put_word(0x1000,  0,(vector<<2) + 2);       // KCS segment=0x1000 for 64KB, 0x0800 for 32KB
}

main()
{
  printf("\nMTX starts in main()\n");
  init();                                       // initialize and create P0 as running
  set_vector(80, int80h);

  kfork("/bin/u1");                             // P0 kfork() P1

  while(1){
    printf("P0 running\n");
    if (proc[1].status != READY && nproc <= 2)  // problem if kill proc 1
      printf("no runable process, system halt\n");
    while(!readyQueue);
    printf("P0 switch process\n");
    tswitch();                                  // P0 switch to run P1
  }
}
