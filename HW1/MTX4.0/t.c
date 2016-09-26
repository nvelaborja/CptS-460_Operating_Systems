/************* tc.c file ******************/
#define SSIZE  1024

typedef struct proc{
        struct proc *next;
               int  *ksp;
               int   kstack[SSIZE];
}PROC;

PROC proc, *running;
int procSize = sizeof(PROC);

int scheduler()
{ 
  running = &proc;
}

main()
{
  printf("in main\n");
  running = &proc;
  printf("call tswitch()\n");
     tswitch();
  printf("back to main()\n");
}

