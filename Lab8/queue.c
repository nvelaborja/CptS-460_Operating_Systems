
/*
 * enter p into queue by priority
 */
int enqueue(PROC **queue, PROC *p)
{
    PROC *pcur = *queue, *pprev = 0;

    if (pcur == 0)              // queue is empty
    {
        *queue = p;
        p->next = 0; return 0;
    }

    if (p->priority > pcur->priority) // the first item (pcur) has the highest priority
    {
        p->next = *queue;
        *queue = p; return 0;
    }

    // until at the end of the queue or p's priority <= the current process' priority
    while((pcur->next) && (pcur->priority >= p->priority))
    {
        pprev = pcur;
        pcur = pcur->next;
    }

    if (pcur->priority < p->priority) // check if the current in queue has lower priority
    {
        pprev->next = p;              // if so insert directly before it
        p->next = pcur;
    }
    else                  // at the end of queue
    {
        pcur->next = p;
        p->next = 0;
    }
    return 0;
}

/*
 * return first element removed from queue
 */
PROC *dequeue(PROC **queue)
{
    PROC *p = *queue;
    if (*queue != 0)            // queue is not empty
    {
        *queue = p->next;    // set the next PROC as the head
    }
    return p;
}

/*
 * print name = list contents
 */
void printList(char *name, PROC *list)
{
    PROC *p = list;
    prints(name);
    while (p != 0)
    {
        printf("%d --> ", p->pid);
        p = p->next;
    }
    printf("NULL\n");
}

/*
 * return a FREE PROC pointer from list
 */
PROC *get_proc(PROC **list)
{
    if (*list != 0)
    {
        return dequeue(&(*list));
    }
    return *list;
}

/*
 * enter p into end of list
 */
int put_proc(PROC **list, PROC *p)
{
    PROC *pcur = *list;

    p->next = 0;

    if (pcur == 0)
    {
        *list = p;
        return 1;
    }
    else
    {
        while (pcur->next != 0)
        {
            pcur = pcur->next;
        }
        // at the end
        pcur->next = p;
    }
}

/*
 * return a FREE PROC pointer from list
 */
PROC *get_proc_sleep(PROC **list, int pid)
{
    PROC *pcur = *list, *pprev = 0;

    if (pcur == 0) // was not found or empty
        return 0;

    while (pcur != 0 && pcur->pid != pid)
    {
        pprev = pcur;
        pcur = pcur->next;
    }

    if (pprev == 0) //first item
    {
        *list = pcur->next;
        return pcur;
    }

    pprev->next = pcur->next;
    return pcur;
}
