// serial.c file for SERIAL LAB ASSIGNEMNT
/**************** CONSTANTS ***********************/
#define BUFLEN      64
#define NULLCHAR     0

#define BEEP          7
#define BACKSPACE     8
#define ESC          27
#define SPACE        32

#define NR_STTY      2    /* number of serial ports */

/* offset from serial ports base */
#define DATA         0   /* Data reg for Rx, Tx   */
#define DIVL         0   /* When used as divisor  */
#define DIVH         1   /* to generate baud rate */
#define IER          1   /* Interrupt Enable reg  */
#define IIR          2   /* Interrupt ID rer      */
#define LCR          3   /* Line Control reg      */
#define MCR          4   /* Modem Control reg     */
#define LSR          5   /* Line Status reg       */
#define MSR          6   /* Modem Status reg      */

/**** The serial terminal data structure ****/

struct stty {
   /* input buffer */
   char inbuf[BUFLEN];
   int inhead, intail;
   struct semaphore inchars, inmutex;

   /* output buffer */
   char outbuf[BUFLEN];
   int outhead, outtail;
   struct semaphore outroom, outmutex;
   int tx_on;

   /* Control section */
   char echo;   /* echo inputs */
   char ison;   /* on or off */
   char erase, kill, intr, quit, x_on, x_off, eof;
   
   /* I/O port base address */
   int port;
} stty[NR_STTY];


/********  bgetc()/bputc() by polling *********/
int bputc(int port, int c)
{
    while ((in_byte(port+LSR) & 0x20) == 0);
    out_byte(port+DATA, c);
}

int bgetc(int port)
{
    while ((in_byte(port+LSR) & 0x01) == 0);
    return (in_byte(port+DATA) & 0x7F);
}

int enable_irq(u8 irq_nr)
{
   out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));
}

   
/************ serial ports initialization ***************/
char *p = "\n\rSerial Port Ready\n\r\007";

int sinit()
{
  int i;  
  struct stty *t;
  char *q; 

  /* initialize stty[] and serial ports */
  for (i = 0; i < NR_STTY; i++){
    q = p;
    printf("sinit : port #%d\n",i);

      t = &stty[i];

      /* initialize data structures and pointers */
      if (i==0)
          t->port = 0x3F8;    /* COM1 base address */
      else
          t->port = 0x2F8;    /* COM2 base address */  

      t->inchars.value  = 0;  t->inchars.queue = 0;
      t->inmutex.value  = 1;  t->inmutex.queue = 0;
      t->outmutex.value = 1;  t->outmutex.queue = 0;
      t->outroom.value = BUFLEN; t->outroom.queue = 0;

      t->inhead = t->intail = 0;
      t->outhead =t->outtail = 0;

      t->tx_on = 0;

      // initialize control chars; NOT used in MTX but show how anyway
      t->ison = t->echo = 1;   /* is on and echoing */
      t->erase = '\b';
      t->kill  = '@';
      t->intr  = (char)0177;  /* del */
      t->quit  = (char)034;   /* control-C */
      t->x_on  = (char)021;   /* control-Q */
      t->x_off = (char)023;   /* control-S */
      t->eof   = (char)004;   /* control-D */

    lock();  // CLI; no interrupts

      out_byte(t->port+MCR,  0x09);  /* IRQ4 on, DTR on */ 
      out_byte(t->port+IER,  0x00);  /* disable serial port interrupts */

      out_byte(t->port+LCR,  0x80);  /* ready to use 3f9,3f8 as divisor */
      out_byte(t->port+DIVH, 0x00);
      out_byte(t->port+DIVL, 12);    /* divisor = 12 ===> 9600 bauds */

      /******** term 9600 /dev/ttyS0: 8 bits/char, no parity *************/ 
      out_byte(t->port+LCR, 0x03); 

      /*******************************************************************
        Writing to 3fc ModemControl tells modem : DTR, then RTS ==>
        let modem respond as a DCE.  Here we must let the (crossed)
        cable tell the TVI terminal that the "DCE" has DSR and CTS.  
        So we turn the port's DTR and RTS on.
      ********************************************************************/

      out_byte(t->port+MCR, 0x0B);  /* 1011 ==> IRQ4, RTS, DTR on   */
      out_byte(t->port+IER, 0x01);  /* Enable Rx interrupt, Tx off */

    unlock();
    
    enable_irq(4-i);  // COM1: IRQ4; COM2: IRQ3

    /* show greeting message */
    //printf("%x\n", t->port);
    while (*q){
      bputc(t->port, *q);
      //putc(*q);
      q++;
    }
  }
}  
         
//======================== LOWER HALF ROUTINES ===============================
int s0handler()
{
  shandler(0);
}
int s1handler()
{
  shandler(1);
}

int shandler(int port)
{  
   struct stty *t;
   int IntID, LineStatus, ModemStatus, intType, c;

   t = &stty[port];            /* IRQ 4 interrupt : COM1 = stty[0] */

   IntID     = in_byte(t->port+IIR);       /* read InterruptID Reg */
   LineStatus= in_byte(t->port+LSR);       /* read LineStatus  Reg */    
   ModemStatus=in_byte(t->port+MSR);       /* read ModemStatus Reg */

   intType = IntID & 7;     /* mask out all except the lowest 3 bits */
   switch(intType){
      case 6 : do_errors(t);  break;   /* 110 = errors */
      case 4 : do_rx(t);      break;   /* 100 = rx interrupt */
      case 2 : do_tx(t);      break;   /* 010 = tx interrupt */
      case 0 : do_modem(t);   break;   /* 000 = modem interrupt */
   }
   out_byte(0x20, 0x20);     /* reenable the 8259 controller */ 
}

int do_errors()
{ printf("assume no error\n"); }

int do_modem()
{  printf("don't have a modem\n"); }


/* The following show how to enable and disable Tx interrupts */

enable_tx(struct stty *t)
{
  lock();
  out_byte(t->port+IER, 0x03);   /* 0011 ==> both tx and rx on */
  t->tx_on = 1;
  unlock();
}

disable_tx(struct stty *t)
{ 
  lock();
  out_byte(t->port+IER, 0x01);   /* 0001 ==> tx off, rx on */
  t->tx_on = 0;
  unlock();
}

// ============= Input Driver ==========================
int do_rx(struct stty *tty)   /* interrupts already disabled */
{ 
  int c;
  c = in_byte(tty->port) & 0x7F;  /* read the ASCII char from port */
  //printf("port %x interrupt:c=%c ", tty->port, c);

  //use bputc() to ECHO the input char to serial port 
  //bputc(tty->port, c);

  if (tty->inchars.value >= BUFLEN)
  {
    bputc(tty->port, BEEP);
    return;
  }

  // if (c==0x3)   // Control-C
  // {
  //   // send SIGINT(2)
  //   c = '\n';
  // }

  if (c == '\r')
  {
    c = '\n';
    bputc(tty->port, c);
  }
  else
    bputc(tty->port, c);

  // Write code to put c into inbuf[ ]; notify process of char available;
  tty->inbuf[tty->inhead++] = c;
  tty->inhead %= BUFLEN;
  V(&tty->inchars);
}      

//----------- UPPER half functions ------------------------   
int sgetc(struct stty *tty)
{ 
  int c;
  // write Code to get a char from inbuf[ ]
  
  P(&tty->inchars); // wait for input key in necessary // wait if no input char
  lock();
    c = tty->inbuf[tty->intail++]; // get a char c from inbuf[ ]
    tty->intail %= BUFLEN;
  unlock();
  return(c); // return char;
}

// int sgetline(struct stty *tty, char *line)
// {  
//    // write code to input as line from tty's inbuf[ ] 
//   printf("sgetline ");
//   P(&tty->inchars);
//     *line = sgetc(tty);
//     while (*line != '\n')
//     {
//       line++;
//     }
//   V(&tty->inchars);
//   return strlen(line);
// }

int sgetline(int port, char *line)
{  
  struct stty *tty = &stty[port];
   // write code to input as line from tty's inbuf[ ] 
  int i;
  //for (i = 0; i < 10; i++)
    //printf("%d : %c\n", i, tty->inbuf[i]);

  printf("sgetline ");
  P(&tty->inmutex);
    while (1) // will need to fix in case other processes might want to print a line
    {
      *line = sgetc(tty);
      if (*line == '\n')
      {
        break;
      }
      //printf("char is %c\n", *line);
      line++;
    }
    *line = NULLCHAR; // replace \n with null terminating
  V(&tty->inmutex);
  return strlen(line);
}


//****************** Output driver *****************************
int do_tx(struct stty *tty)
{
  int c;
  //printf("tx interrupt ");
  if (tty->outroom.value == BUFLEN){ // nothing to do 
     disable_tx(tty);                 // turn off tx interrupt
     return;
  }

  // write code to output a char from tty's outbuf[ ]
  //       out_byte(tty->port, c); will output c to port

  if (tty->outroom.value < BUFLEN)
  {
    c = tty->outbuf[tty->outtail++];
    tty->outtail %= BUFLEN;
    out_byte(tty->port, c);
    V(&tty->outroom);
  }
}


//--------------- UPPER half functions -------------------
int sputc(struct stty *tty, int c)
{
  // write code to put c into tty's outbuf[ ]

  P(&tty->outroom);                // WAIT FOR space in tty's outbuf[];
  lock();                           // disable interrupts
    tty->outbuf[tty->outhead++] = c; //enter c into tty's outbuf[ ];
    tty->outhead %= BUFLEN;
    if (!tty->tx_on)
      enable_tx(tty);               //enable tx interrupt if it is off;
  unlock();                         //enable interrupts
}

// int sputline(struct stty *tty, char *line)
// {
//   // write code to output a line to tty
// }

int sputline(int port, char *line)
{
  // write code to output a line to tty
  struct stty *tty = &stty[port];

  /* Will need to handle multi processes later */
  P(&tty->outmutex);
    while (*line)
    {
      sputc(tty, *line++);   // print chars
    }
    sputc(tty, '\n');       // just so that it goes to next line
  V(&tty->outmutex);
}

/*********************** Kernel calls ***************************/
int do_sgetline()
{
  char *port, line[64];

  printf("Enter port number [0|1] : ");
  gets(port);

  sgetline(atoi(port), line);
  printf("PORT %d wrote: %s\n", atoi(port), line);
}

int do_sputline()
{
  char line[64], *port;

  printf("Enter port number [0|1] : ");
  gets(port);

  printf("input a line to output: ");
  gets(line);

  sputline(atoi(port), line);
}
