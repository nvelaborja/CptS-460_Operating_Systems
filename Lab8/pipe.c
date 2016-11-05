
char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

display_pipe(PIPE *p)
{
    int i, j = p->tail; // tail is used to read
    printf("------------ PIPE CONTENETS ------------\n");
    printf("nreader=%d  nwriter=%d  ", p->nreader, p->nwriter);
    printf("data=%d room=%d\n",p->data, p->room);
    printf("contents=");
    for (i=p->data; i>0; i--)
    {
        printf("%c", p->buf[j++]);
        j %= PSIZE;             // so that it wraps around
    }
    printf("\n------------------------------------\n");
}

int pfd()
{
    int i;
    int valid_fds = 0;

    for (i =0; i<NFD; i++)
    {
        if (running->fd[i])
            valid_fds = 1;
    }

    if (valid_fds == 0)
    {
        printf("no opened fds\n");
        return -1;
    }

    printf("========== valid fd ==========\n");
    for (i =0; i<NFD; i++)
    {
        if (running->fd[i])
            printf("%d   %s   refCount = %d\n", i, MODE[running->fd[i]->mode], running->fd[i]->refCount);
    }
    printf("==============================\n");
}

PIPE * allocate_pipe()
{
    int i;
    PIPE *p;

    /* Allocate a PIPE object.  */
    while (i < NPIPE)
    {
        if (pipe[i].busy == 0)
            break;
        i++;
    }
    // Initialize the PIPE object with head=tail=0; data=0; room=PSIZE; nreader=nwriters=1;
    pipe[i].busy = 1;
    p = &pipe[i];
    p->head = p->tail = 0;
    p->data = 0;
    p-> room = PSIZE;
    p->nreader = p->nwriter = 0;
    return p;
}

OFT * allocate_oft()
{
    int i;

    for (i=0; i<NOFT; i++)
    {
        if (oft[i].refCount == 0)
            break;
    }
    return &oft[i];
}

int kpipe(int pd[2])
{
    int i, j;
    PIPE *p;
    OFT *oft1, *oft2;

    /* Allocate a PIPE object.  */
    p = allocate_pipe();

    /* Allocate 2 OFTs. Initialize the OFTs as
     * readOFT.mode = READ_PIPE; writeOFT.mode = WRITE_PIPE;
     * both OFT's refCount = 1 and pipe_ptr points to the same PIPE object
     */
    oft1 = allocate_oft();
    oft1->refCount = 1;

    oft2 = allocate_oft();
    oft2->refCount = 1;

    oft1->mode = READ_PIPE;
    oft2->mode = WRITE_PIPE;
    oft1->pipe_ptr = oft2->pipe_ptr = p;
    p->nreader = p->nwriter = 1;            // need to now set pipe's nreader and nwriter to 1 for each

    /* Allocate 2 free entries in the PROC.fd[] array, e.g. fd[i] and fd[j]
     * Let fd[i] point to readOFT and fd[j] point to writeOFT
     */
    for (i=0; i<NFD; i++)
    {
        j = i + 1;
        if (running->fd[i] == 0 && running->fd[j] == 0)
            break;
    }
    running->fd[i] = oft1;
    running->fd[j] = oft2;

    /* Write index i to pd[0] and index j to pd[1]; both are in Uspace */
    put_word(i, running->uss, pd[0]);
    put_word(j, running->uss, pd[1]);

    /* return 0 for OK */
    printf("proc %d now has file descriptors [%d %d]\n", running->pid, i, j);
    return 0;
}

int close_pipe(int fd)
{
    OFT *ofd;
    PIPE *p;

    /* 1. Validate fd to make sure its a valid opened file descriptor*/
    if (running->fd[fd] == 0 || fd < 0 || fd > NFD)
    {
        printf("invalid fd\n");
        return -1;
    }

    /* 2. Follow PROC.fd[fd] to its OFT. Decrement OFT.refCount by 1*/
    ofd = running->fd[fd];
    ofd->refCount--;

    /* 3. */
    if (ofd->mode == WRITE_PIPE)
    {
        p = ofd->pipe_ptr;

        p->nwriter--;            // decrement nwriter by 1

        if (p->nwriter == 0)     // last writer on pipe
        {
            if (p->nreader == 0) // no more readers
            {
                //deallocate pipe
                p->busy = 0; return;
            }
            // deallocate writeOFT
        }
        kwakeup(&p->data);       // wakeup ALL blocked readers
    }

    /* 4. */
    else    // READ_PIPE
    {
        p = ofd->pipe_ptr;

        p->nreader--;            // decrement nreader by 1

        if (p->nreader == 0)     // last writer on pipe
        {
            if (p->nwriter == 0) // no more readers
            {
                //deallocate pipe
                p->busy = 0; return;
            }
            // deallocate readOFT
        }
        kwakeup(&p->room);       // wake up ALL blocked writers
    }

    /* 5. Clear caller's fd[fd] to 0; return OK; */
    running->fd[fd] = 0;
    return 0;
}

int read_pipe(int fd, char *buf, int n)
{
    int r = 0;
    OFT *ofd;
    PIPE *p;

    if (n<=0)
        return 0;

    /* validate fd; from fd, get OFT and pipe pointer p */
    if (running->fd[fd] == 0 || fd < 0 || fd > NFD)
    {
        printf("invalid fd\n");
        return -1;
    }
    ofd = running->fd[fd];
    p = ofd->pipe_ptr;

    printf("pipe before reading\n");
    display_pipe(p);

    while (n){
        while (p->data) {
            // read a byte from pipe to buf
            put_byte(p->buf[p->tail], running->uss, buf); // tail is used to read
            n--; r++; p->data--; p->room++; p->tail++; buf++;
            p->tail %= PSIZE;       // when increment mod by size to make circular
            if (n == 0)
                break;
        }
        if (r) {                // has read some data
            printf("pipe after reading\n");
            display_pipe(p);
            kwakeup(&p->room);
            return r;
        }
        // pipe has no data
        if (p->nwriter) {      // if pipe still has writer
            printf("pipe before reader goes to sleep\n");
            display_pipe(p);

            kwakeup(&p->room);  // wakeup ALL writers, if any
            ksleep(&p->data);   // sleep for data
            continue;
        }
        // pipe has no writer and no data
        return 0;
    }
}

int write_pipe(int fd, char *buf, int n)
{
    int r = 0;
    OFT *ofd;
    PIPE *p;

    if (n<=0)
        return 0;

    /* validate fd; from fd, get OFT and pipe pointer p */
    if (running->fd[fd] == 0 || fd < 0 || fd > NFD)
    {
        printf("invalid fd\n");
        return -1;
    }
    ofd = running->fd[fd];
    p = ofd->pipe_ptr;

    printf("pipe before writing\n");
    display_pipe(p);

    while (n){
        if (!p->nreader)        // no more readers
            kexit(BROKEN_PIPE);   // BROKEN_PIPE error

        while (p->room) {
            // write a byte from buf to pipe
            p->buf[p->head] = get_byte(running->uss, buf);      // head is used to write
            r++; p->data++; p->room--; n--; p->head++; buf++;
            p->head %= PSIZE;       // when increment mod by size to make circular
            if (n == 0)
                break;
        }
        kwakeup(&p->data);          // wakeup ALL readers, if any
        if (n==0)
        {
            printf("pipe after writing\n");
            display_pipe(p);

            return r;               // finished writing n bytes
        }
        // still has data to write but pipe has no room
        printf("pipe before writer goes to sleep\n");
        display_pipe(p);
        kwakeup(&p->data);           // wake up readers
        ksleep(&p->room);           // sleep for room
    }
}

