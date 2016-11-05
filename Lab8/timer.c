/******************** timer.c file *************************************/
#define LATCH_COUNT     0x00	   /* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	   /* ccaammmb, a = access, m = mode, b = BCD */

#define TIMER_FREQ   1193182L	   /* clock frequency for timer in PC and AT */
#define TIMER_COUNT  TIMER_FREQ/60 /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0

u16 tick;

typedef struct tq {
  struct tq *next;    // next element pointer
  int       time;     // requested time
  PROC      *proc;  // pointer to PROC
  int    (*action)(); // 0|1|handler function pointer
} TQE;
TQE *tq, tqe[NPROC];  // tq   timer queue pointer

// in serial.c
// int enable_irq(u16 irq_nr)
// {
//   lock();
//     out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));
// }

int timer_init()
{
  int i;
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */

  printf("timer init\n");
  tick = 0; 
  out_byte(TIMER_MODE, SQUARE_WAVE);	// set timer to run continuously
  out_byte(TIMER0, TIMER_COUNT);	// timer count low byte
  out_byte(TIMER0, TIMER_COUNT >> 8);	// timer count high byte 
  enable_irq(TIMER_IRQ); 

  for (i=0; i<NPROC; i++)
    tqe[i].next = &tqe[i+1];
  tqe[NPROC-1].next = 0;
}

int hh=0, mm=0, ss=0;
int wall_clock()
{
  int orig_row, orig_col, i;

  orig_row = row; orig_col = column;    // save where the cursor was at
                            // increment seconds
  if (ss != 0 && ss % 60 == 0)
  {
    mm++;                             // increment minutes
    if (mm % 60 == 0)
    {
      hh++;                           //increment hours
    }
  }
  //printf ("row %d, col %d\n", row, column);

  row = 24; column = 65;
  move_cursor();
  color = 0x0f;   // black bg, white font
  
  if (hh < 10)
    putc('0');
  printf("%d:", hh % 24);

  if (mm % 60 < 10)
    putc('0');
  printf("%d:", mm % 60);

  if (ss % 60 < 10)
    putc('0');
  printf("%d", ss % 60);

  row = orig_row; column = orig_col;    // reset back to where cursor was at
  if (row == 23) { row = 24; }          // so it doesnt overwrite the last row when called right after clear_clock
  move_cursor();
}

/*===================================================================*
 *		    timer interrupt handler       		     *
 *===================================================================*/
int enable_timeslice = 0;   // just a flag to enable time slicing (Task 2)
int thandler()
{ 
  TQE* tp;
  tick++; 
  tick %= 60;

  if (tick == 0){                      // at each second
    
    ss++;    
    wall_clock();
    
    if (running->inkmode <= 1 && enable_timeslice == 1)             // On each second, decrement running's time in Umode only
    {
      running->time--;
    }

    /* itimer */
    if (tq != 0)
    {
      tp = tq;
      while (tp != 0)
      {
        tp->time--;
        printTQList();
        
        if (tp->time > 0) // do it every second and break if it hasnt reached the end of timer
          break;

        printf("itimer for proc %d done. Waking up\n", tq->proc->pid);
        kwakeup(&tp->proc->time);
        tq = tp = tp->next;
      }
    }
  }
  out_byte(0x20, 0x20);                // tell 8259 PIC EOI

  if (running->time <= 0 && running->inkmode <= 1 && enable_timeslice == 1)
  {
    printf("PROC %d time_slice=%d, process switch\n", running->pid, running->time);
    running->time = TSLICE;
    tswitch();
  }
}

int printTQList()
{
   TQE *t;
   t = tq;
   printf("timerQueue = ");
   while(t != 0){
      printf(" [P%d:%d] -> ", t->proc->pid, t->time);
      t = t->next;
   }
   printf("\n");
}

// int thandler()
// {
//   tick++; 
//   tick %= 60;
//   if (tick == 0){                      // at each second
//       printf("1 second timer interrupt in ");
//       running->inkmode > 1 ? putc('K') : putc('U');
//       printf("mode\n");
//   }
//   out_byte(0x20, 0x20);                // tell 8259 PIC EOI
// }

int tqcount = 0;
TQE *get_tqe_node()
{
  TQE *t;
  t = &tqe[tqcount]; // circular queue array
  tqcount++;
  if (tqcount == 8)
    tqcount = 0;
  t->next = &tqe[tqcount];    // set the last to the next 
  return t;
}

int itimer(int time) 
{
  /*(1) Fill in TQE[pid] information, e.g. proc pointer, action*/
  TQE *t;

  t = get_tqe_node();
  t->time = time;
  t->proc = running;
  t->action = ksleep;

  printf("In itimer: proc=%d timer at %d\n", t->proc->pid, t->time);

  /*(2) lock() // mask out interrupts */
  lock();

  /* (3) & (4) */
  enqueue_tq_node(t, time);

  printf("PROC %d is going to sleep\n", running->pid);
  //ksleep(&running->time);  // sleep on its own time address
  (t->action)(&running->time);

  /*(5) unlock() // unmask interrupts*/
  unlock();
}

int enqueue_tq_node(TQE *t, int time)
{
  TQE *curr, *prev;
  
  /*(3) traverse timer queue to compute the position to insert TQE*/
  if (tq == 0) // timer requeust queue is empty
  {
    tq = t;
    t->next = 0;
  }
  else
  {
    curr = tq;
    while (curr != 0)
    { 
      if (time - curr->time < 0) // currents time is greater so must insert before
          break;  
      time -= curr->time;
      prev = curr;
      curr = curr->next;
    }

    /*(4) insert the TQE and update the time of next TQE;*/
    t->time = time; // set the relative time

    if (curr != 0) // not the end
    { 
      curr->time -= time; // refresh relative time
    }
    
    if (curr == tq) // at the front
    {
      t->next = tq;
      tq = t;
    }
    else // in the middle
    {
      t->next = curr;
      prev->next = t;
    }
  }
}

/* Called in scroll() in vid.c */
int clear_clock()
{
  int org_color = color;
  row = 23; column = 65;
  color = 0x00;
  printf("00 :00 :00 ");
  column -= 8;
  color = org_color;
}