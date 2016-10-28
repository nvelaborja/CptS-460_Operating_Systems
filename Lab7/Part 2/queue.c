extern PROC *readyQueue, *freeList;

/****************** utility.c file ***********************/
int put_proc(PROC **list, PROC *p) //puts a proc back in the freeList
{
	//printf("P%d Enter put_proc \n", running->pid);
    p->status = FREE;
	p->next = *list;
	*list = p;	
}

PROC *get_proc(PROC **list) //grabs a proc from freeList
{
	PROC *p;

	if(*list == 0){ //null address
		printf("get_proc, null\n");
		return 0;
	}
	p = *list;
    *list = p->next; //update root
	return p;
}

int enqueue(PROC **queue, PROC *p)
{
	PROC *previous, *current;

//printf("P%d Enter enqueue \n", running->pid);

	//break it down by cases
	//*queue references first node in the linked list queue

	//if new node has highest priority
	if (p->priority > (*queue)->priority)
	{
		//add queue to back of node, then make node queue
		p->next = *queue;
		*queue = p;
	}

	//if queue is empty
	else if (*queue == 0)
	{
		*queue = p;
		(*queue)->next = 0;
	}

	//node isn't highest priority and queue exists
	else
	{
		current = *queue;
		
		//while nodes exist in the queue and p prior isn't greater
		while (current && p->priority <= current->priority)
		{
			//move down the queue
			previous = current;
			current = current->next;
		}

		//if we reach the end
		if (current == 0)
		{
			//staple p onto the end of the list
			previous->next = p;
			p->next = 0;
		}

		//if we are in the middle
		else
		{
			//add p to end of current, add previous to end of p
			p->next = current;
			previous->next = p;
		}
	}

	return 0;
}

PROC *dequeue(PROC **queue)
{
	PROC *p;
	p = *queue; 					// pop first element into p

	//printf("P%d Enter dequeue \n", running->pid);

	if (p != 0)
	{
		*queue = (*queue)->next; 	// if it worked, move queue
	}

	return p;
}

void MYprintList(PROC *queue){
	if(queue == 0){
		printf("NULL\n");
		return;
	}

	printf("[pid = %d, priority = %d] -> ", queue->pid, queue->priority);

	printList(queue->next);
}

// print queue and print list functions 
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
