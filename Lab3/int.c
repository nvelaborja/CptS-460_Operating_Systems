
/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/

// Define relative offset for each item in the user stack
#define UDS   0
#define UES   1
#define UDI   2
#define USI   3
#define UBP   4
#define UDX   5
#define UCX   6
#define UBX   7
#define UAX   8
#define UPC   9
#define UCS   10
#define UFLAG 11
#define RETPC 12
#define A     13
#define B     14
#define C     15
#define D     16

char kkgetc();

/****************** syscall handler in C ***************************/
int kcinth()
{
   int a, b, c, d, r;
   u16 segment, offset;

   //printf("P%d Enter kcinth \n", running->pid);

   segment = running->uss;
   offset = running->usp;

	// WRITE CODE TO GET get syscall parameters a,b,c,d from ustack 
   a = get_word(segment, offset + 2 * A);
   b = get_word(segment, offset + 2 * B);
   c = get_word(segment, offset + 2 * C);
   d = get_word(segment, offset + 2 * D);

   switch(a){
       case 0 : r = kgetpid();        break;
       case 1 : r = kps();            break;
       case 2 : r = kchname(b);       break;
       case 3 : r = kkfork();         break;
       case 4 : r = ktswitch();       break;
       case 5 : r = kkwait(b);        break;
       case 6 : r = kkexit(b);        break;
       case 7 : r = getc();		      break;	// Add getc and putc
       case 8 : r = putc(b);		    break;

       case 99: kkexit(b);            break;
       default: printf("Invalid Syscall # : %d\n", a); 
   }

	// WRITE CODE to let r be the return value to Umode
   put_word(r, segment, offset + 2 * UAX);
}

//============= WRITE C CODE FOR syscall functions ======================

int kgetpid()
{
    //WRITE YOUR C code
    //printf("P%d Enter kgetpid \n", running->pid);
    return running->pid;
}

int kps()
{
    //WRITE C code to print PROC information
    int i, j;
    PROC *p;

    //printf("P%d Enter kps \n", running->pid);
    
    // Print header
    printf("*******************************************************\n");
    printf("|     Name          Status        PID       PPID      |\n");
    printf("|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|\n");

    // Loop and print the rest
    for (i = 0; i < NPROC; i++)
    {
    	p = &proc[i];

    	printf("|   %s          ", p->name);

    	switch(p->status)
    	{
    		case FREE: printf("FREE"); break;
    		case READY: printf("READY"); break;
    		case RUNNING: printf("RUNNING"); break;
    		case STOPPED: printf("STOPPED"); break;
    		case SLEEP: printf("SLEEP"); break;
    		case ZOMBIE: printf("ZOMBIE"); break;
    		default: printf("NULL"); break;
    	}

    	if (p->status != FREE)
    		printf("          %d           %d     |\n", p->pid, p->ppid);
    	else
    		printf("         NULL         NULL    |\n");
    }
    printf("*******************************************************\n\n");

    return 0;
}

int kchname(char *name)
{
    //WRITE C CODE to change running's name string;
    PROC *p = running;
    char buffer[32];
    char *cp;
	  int i = 0;

	  //printf("P%d Enter kchname \n", running->pid);
    
    while (i < 32)      // Go to user space and get the name
    {
      buffer[i] = get_word(running->uss, name + i);
      i++;
    }

    buffer[31] = 0;             // Terminate it

    printf("Attempting to change name of P%d to %s.\n", p->pid, buffer);

    /*if (strlen(name) > 31)					// Check to see name will fit in struct
    {
    	printf("Filename too long.\n");
    	return -1;
    }*/

    strcpy(p->name, buffer);					// If good, strcpy that baby in there

    return 1;
}

int kkfork()
{
  	//use you kfork() in kernel;
  	//return child pid or -1 to Umode!!!
	PROC *p;

	//printf("P%d Enter kkfork \n", running->pid);

	p = kfork("/bin/u1");

	if (!p) return -1;

	return p->pid;
}

int ktswitch()
{
	//printf("P%d Enter ktswitch \n", running->pid);
    return tswitch();
}

int kkwait(int *status)
{

  //use YOUR kwait() in LAB3;
  //return values to Umode!!!
	//printf("P%d Enter kkwait \n", running->pid);
	return kwait(status);
}

int kkexit(int value)
{
    //use your kexit() in LAB3
    //do NOT let P1 die
    //printf("P%d Enter kkexit \n", running->pid);
    return kexit(value);	// kexit makes sure P1 doesn't die
}
