/******************** timer.c file *************************************/
#define LATCH_COUNT     0x00          /* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36          /* ccaammmb, a = access, m = mode, b = BCD */

#define TIMER_FREQ   1193182L         /* clock frequency for timer in PC and AT */
#define TIMER_COUNT  TIMER_FREQ/60      /* initial value for counter*/

#define TIMER_PORT      0x40        //channel 0 timer port address
#define TIMER_MODE    0x43
#define TIMER_IRQ          0        // IRQ0
#define INT_CNTL    0x20        //master PIC control register
#define INT_MASK    0x21        //master PIC mask reg:bit i=0=enable

typedef struct tq
{
  struct tq *next;    //next element pointer
  int      time;      //Requested timer
  struct PROC *proc;  //Pointer to proc
  int   (*action)();  // 0|1|handler function pointer
}TQE;

TQE *tq, tqe[NPROC];

u16 tick;
u16 sec;                    // Timer seconds counter
u16 min;                    // Timer minutes counter
u16 hr;                     // Timer hours counter

int enable_irq(u16 irq)
{
  lock();
  out_byte(INT_MASK, in_byte(INT_MASK) & ~(1 << irq));

}

int timer_init()
{
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */

  printf("timer init\n");
  tick = 0;
  out_byte(TIMER_MODE, SQUARE_WAVE);    // set timer to run continuously
  out_byte(TIMER_PORT, TIMER_COUNT);      // timer count low byte
  out_byte(TIMER_PORT, TIMER_COUNT >> 8);   // timer count high byte 
  enable_irq(TIMER_IRQ);

  // Initialize clock values
  sec = min = hr = 0;

  printTime();
}

int printTime()//implements clock in bottom right corner
{
  int orig_row = row, orig_col = column;
  int i;
  char time[8];
  
  row = 24; column = 71;

  color = running->pid + 0x0A;

  move_cursor();

  time[0] = '0' + ((int)(hr / 10) % 10);
  time[1] = '0' + (hr % 10);
  time[2] = ':';
  time[3] = '0' + ((int)(min / 10) % 10);
  time[4] = '0' + (min % 10);
  time[5] = ':';
  time[6] = '0' + ((int)(sec / 10) % 10);
  time[7] = '0' + (sec % 10);
  
  for (i = 0; i < 8; i++)
  {
    putc(time[i]);
  }

  row = orig_row; column = orig_col;
  move_cursor();

}

int eraseTime()
{
  int orig_row = row, orig_col = column;
  int i;
  
  row = 23; column = 71;
  move_cursor();

  for (i = 0; i < 8; i++)
      putc(' ');

  row = orig_row; column = orig_col;
  move_cursor();
}


int printTQ()
{
  TQE *temp = tq;
  PROC *p = temp->proc;

  if(temp == 0)
  {
    printf("\nNULL\n");
    return 0;
  }
  while(temp != 0)
  {
    printf("\ntimerQueue = [P%d, %d] -> ", p->pid, temp->time);
    temp = temp->next;
    p = temp->proc;
  }
  printf("NULL\n");

  // printf("tq->time = %d\n", tq->time);
  // printf("tq->proc->pid = %d\n", p->pid);
}
int itimer(int t)
{
  int f = 1;
  TQE *cur = tq; TQE *prev = tq;
  tqe[running->pid].proc = running;
  printf("Running pid = %d", running->pid);
  lock();
  if(tq == 0)         //if tq is empty
  {
    tq = &tqe[running->pid];
    tqe[running->pid].time = t;
    tqe[running->pid].next = 0;
  }
  else
  {
    while(cur) //while we have a valid pointer
    {
      if((t - cur->time) < 0)
      {
        break;
      }
      t -= cur->time;
      prev = cur;
      cur = cur->next;
    }
    if(cur) //if we insert in middle or at front.
    {
      cur->time -= t; 
    }
    tqe[running->pid].time = t;
    if(cur == tq)//at the first of the list
    {
      tqe[running->pid].next = tq;
      tq = &tqe[running->pid];
    }
    else
    {
      tqe[running->pid].next = cur;
      prev->next = &tqe[running->pid];
    }
  }
  unlock();
  ksleep(running->pid); //put to sleep
  printTQ();
}

/*===================================================================*
*       timer interrupt handler                *
*===================================================================*/
int thandler()
{
  tick++;
  tick %= 60;
  if (tick == 0) 
  {                       // at each second
    addSecond();
    printTime();

    //printf("inkmord = %d\n", running->inkmode);
    if(running->inkmode <= 1)
    {
      //printf("INUMODE\n");
      if(tq)
      {
        tq->time--;
        printTQ();
        if(tq->time <= 0)
        {
          PROC *p = tq->proc;
          kwakeup(p->pid);
          tq = tq->next;
          tqe[p->pid].proc = 0;   //null out the array spot.
          tqe[p->pid].next = 0;
        }
      }
    }
    else
    {
      //printf("INKMODE\n");
    }
  }
  out_byte(0x20, 0x20);                 // tell 8259 PIC EOI
}

int addSecond()                         // Add a second to our global timer
{
  sec++;

  if (sec > 59)
  {
    sec %= 60;
    addMinute();
  }

  return 1;
}

int addMinute()
{
  min++;

  if (min > 59)
  {
    min %= 60;
    addHour();
  }

  return 1;
}

int addHour()
{
  hr++;

  if (hr > 23)
    hr %= 24;

  return 1;
}