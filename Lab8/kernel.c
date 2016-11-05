

/*************** kernel command functions ***********/
int body()
{
   char c; int exitValue;
   color = 0x0f;
   printf("----------------KERNEL MODE-------------------\n");
   printf("PROC %d resumes from body()\n", running->pid);
   while(1){
     color = 0x0f;// white in kmode//running->pid + 7;
     printf("PROC %d running: parent = %d\n", running->pid, running->ppid);
     printf("------------------------------------------------------\n");
     printList("freelist   => ", freeList); // optional: show the freelist
     printList("readyQueue => ", readyQueue); // show the readyQueue
     printList("sleepList  => ", sleepList);
     printf("------------------------------------------------------\n");
     printf("enter a char [s|f|w|q|u|i|o] : ");
     c = getc(); printf("%c\n", c);
     switch(c)
     {
         case 'f' : fork();       break;//do_kfork();   break;
         case 's' : do_tswitch(); break;
         case 'w' : do_wait();    break;
         case 'q' : printf("enter an exitValue: ");
                    exitValue = gets();
                    do_exit(atoi(exitValue));    break;
         case 'u' : goUmode();    break;
         case 'i' : do_sgetline(); break;
         case 'o' : do_sputline(); break;

         default: printf("invalid command\n"); break;
     }
   }
}

/* create the Umode image from file u1
 * and load into its segment (pg 153)
 */
int makeUserImage(char* filename, PROC *p)
{
    int i, segment = 0x1000*(p->pid + 1);

    if (!load(filename, segment))   // load filename to segment
        return -1;                  // if load failed, return -1 to Umode

    /* re-initialize process ustack for it return to VA=0 */
    for (i = 1; i <= 12; i++) // throught 12 because dont want o overwrite retPC and 1 because dont 0 out Uds
        put_word(0, segment, -2*i);

    /*  -1   -2 -3  -4 -5 -6 -7 -8 -9 -10 -11 -12 */
    /* flag uCS uPC ax bx cx dx bp si  di uES uDs */
    put_word(0x0200,  segment, -2*1);   // Umode flag=0x200
    put_word(segment, segment, -2*2);   // uCS=segment; uPC=0
    put_word(segment, segment, -2*11);  // saved uES=segment
    put_word(segment, segment, -2*12);  // saved uDS=segment

    p->usp = -24;                   // new usp = -2 * 12
    p->uss = segment;               // set at segment
}

PROC *kfork(char *filename) // create a child process, begin with body()
{
    int i, segment;
    PROC *p = get_proc(&freeList); // get proc for child process
    if (!p)
    {
        printf("no more PROC, kfork() failed\n");
        return 0;
    }

    /* initialize new proc and its stack */
    p->status = READY;
    p->priority = 1;            // priority = 1 for all proc except P0
    p->ppid = running->pid;     // parent = running
    p->parent = running;
    p->inkmode = 1;             // iniitilize to kmode
    p->time = 0;

    /* initialize new proc's kstack [] */
    for (i=1; i<10; i++)        // saved CPU registers
    {
        p->kstack[SSIZE - i] = 0;   // all initialized to 0
    }

    p->kstack[SSIZE-1] = (int)goUmode;//(int)body; // resume point = address of body()
    p->ksp = &(p->kstack[SSIZE-9]); // proc saved sp = ksp -> kstack top
    enqueue(&readyQueue, p);        // enter p into readyQueue by priority
    nproc++;
    segment = 0x1000*(p->pid + 1);
    if (filename)
    {
        makeUserImage(filename, p);
        printf("PROC %d kforked a child (proc %d, segment %x)\n", running->pid, p->pid, segment);
    }
    else
        printf("PROC %d kforked a child proc %d, segment %x\n", running->pid, p->pid, segment);
    return p;                       // return child PROC pointer
}

#include "forkexec.c" // Lab 4

int do_kfork()
{
    PROC *p = kfork("/bin/u1");
    if (p == 0) { printf("kfork failed\n"); return -1; }
    return p->pid;
}

int do_tswitch()
{
    printf("proc %d called tswitch()\n", running->pid);
    tswitch();
    printf("proc %d resumes\n", running->pid);
}

int do_sleep(int event)
{ // shown above
    ksleep(event);
}

int do_wakeup(int event)
{ // shown above
    kwakeup(event);
}

int do_exit(int exitValue)
{
    /*char* s; int s_code;
    printf("Enter exit code: ");
    gets(s); putc('\r');
    s_code = atoi(s);*/
    if (running->pid == 1 && nproc > 2)
    {
        printf("! PROC 1 cannot die. It has child procs\n");
        return -1;
    }
    kexit(exitValue);
}

int do_wait()
{
    int exitCode;
    int childPid = kwait(&exitCode);
    if (childPid == -1)
        printf("PROC %d has no children to wait on\n", running->pid);
    else
        printf ("child PROC %d has been laid to rest. child PROC exit code = %d\n", childPid, exitCode);
}
