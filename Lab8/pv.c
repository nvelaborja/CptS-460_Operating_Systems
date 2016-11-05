/* pg 180 */
// int_off ==> { int SR = lock(); } where lock() disables CPU interrups and returns the
// 									original CPU status register

// int_on == > { unlock(SR); }		unlock(SR) restores CPU's status register to its original value

struct semaphore{
  int value;		// initial value of semaphore
  PROC *queue;		// a FIFO queue of the BLOCKed processes
};

int P(struct semaphore *s)
{
  // write YOUR C code for P()
	int_off();

	s->value--;
	if (s->value < 0)
	{
		// block the process in the semaphore's FIFO queue
		running->status = BLOCK;
		enqueue(&s->queue, running);
		tswitch();
	}
	int_on();
}

int V(struct semaphore *s)
{
  // write YOUR C code for V()
	PROC *p;
	int_off();

	s->value++;
	if (s->value <= 0)
	{
		p = dequeue(&s->queue);
		p->status = READY;
		enqueue(&readyQueue, p);
	}
	int_on();
}
