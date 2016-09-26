/******* MTX4.5 kernel queue.c file *******/

void enqueue(PROC **queue, PROC *p)
{
	PROC *current, *next;

	if ((*queue) == 0)				// Empty Queue
	{
		*queue = p;				// Set head to first proc, next to null
		(*queue)->next = 0;
	}

	else if (p->priority > (*queue)->priority)	// New proc with higher priority
	{
		p->next = (*queue);			// Set p's next to current head, set head to p
		(*queue) = p;
	}

	else						// Regular enqueue
	{
		current = (*queue);
		next = current->next;

		while (current != 0 && p->priority <= next->priority)
		{
			current = next;
			next = current->next;
		}

		current->next = p;
		p->next = next;
	}
}

PROC *dequeue (PROC **queue)
{
	PROC *p = *queue;				// Get current head

	if (*queue != 0)				
		*queue = (*queue)->next;

	return p;
}

PROC *get_proc(PROC **list)				// return a FREE PROC pointer from list
{
	if (freeList != 0)
		return dequeue(&freeList);
	return 0;
}

int put_proc (PROC **list, PROC *p)			// enter p into list
{
	// Free the process
	p->status = FREE;

	// Check list for other items, otherwise list = p
	if (freeList == 0)
	{
		freeList = p;
		p->next = 0;
	}
	else
	{
		p->next = freeList->next;
		freeList->next = p;
	}
}

int printList(char *name, PROC *p)
{
   printf("%s = ", name);
   while(p){
     printf("%d-> ", p->pid);
       p = p->next;
   }
   printf("NULL\n");
}

int printQueue(char *name, PROC *p)
{
   printf("%s = ", name);
   while(p){
     printf("%d[%d] -> ", p->pid, p->priority);
       p = p->next;
   }
   printf("NULL\n");
}

int printSleepList(char *name, PROC *p)
{
   printf("%s = ", name);
   while(p){
     printf("%d[ e=%d] -> ", p->pid, p->event);
       p = p->next;
   }
   printf("NULL\n");
}
