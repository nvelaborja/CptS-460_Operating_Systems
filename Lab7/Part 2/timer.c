/******************** timer.c file *************************************/
#define LATCH_COUNT     0x00	   /* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	   /* ccaammmb, a = access, m = mode, b = BCD */

#define TIMER_FREQ   1193182L	   /* clock frequency for timer in PC and AT */
#define TIMER_COUNT  TIMER_FREQ/60 /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0

u16 tick;                   // Timer tick @ 60Hz
u16 sec;                    // Timer seconds counter
u16 min;                    // Timer minutes counter
u16 hr;                     // Timer hours counter

int enable_irq(u16 irq_nr)
{
  lock();
    out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));

}

int timer_init()
{
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */
  int i;

  printf("timer init\n");
  tick = 0; 
  out_byte(TIMER_MODE, SQUARE_WAVE);	// set timer to run continuously
  out_byte(TIMER0, TIMER_COUNT);	// timer count low byte
  out_byte(TIMER0, TIMER_COUNT >> 8);	// timer count high byte 
  enable_irq(TIMER_IRQ); 

  // Initialize clock values
  sec = min = hr = 0;
}

/*===================================================================*
 *		    timer interrupt handler       		     *
 *===================================================================*/
int thandler()
{
  int oldRow, oldColumn, i;
  char time[8];

  tick++; 

  if (tick > 60){                      // at each second
    tick %= 60;
    addSecond();                        // Add a second to our timer
    running->time--;                    // Decrement running proc's switch time

    // Print time in bottom right of screen
    oldRow = row; oldColumn = column;     // Remember previous position so we can move cursor back 

    row = 24; column = 70;
    move_cursor();
    printTime();

    // Reset cursor
    row = oldRow;
    column = oldColumn;
    move_cursor();
  }

  //printf("im: %d | time: %d\n", running->inkmode, running->time);

  out_byte(0x20, 0x20);                // tell 8259 PIC EOI

  if (running->time <= 0)
  {
    tswitch();
  }
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

int printTime(char *time)
{
  int i;

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

  return 1;
}