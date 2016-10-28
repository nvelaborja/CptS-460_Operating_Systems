// pipe.c for EXAM 1 PIPE
// Nathan VelaBorja - 11392441

int kpipe(int pd[2])
{
	PIPE *pipe;
	OFT *oft1, *oft2;
	int i;

	for (i = 0; i < NPIPE && pipes[i].busy; i++);			// Loop through pipes until we find a free one
	pipe = &pipes[i];										// Grab it and initialize it's info
	pipe->busy = 1;
	pipe->head = 0;
	pipe->tail = 0;
	pipe->data = 0;
	pipe->room = PSIZE;

	for (i = 0; i < NOFT && ofts[i].refCount > 0; i++);		// Loop through ofts until we find first free one
	oft1 = &ofts[i];										// Grab it and initialize it's info
	oft1->refCount = 1;
	oft1->mode = READ_PIPE;
	oft1->pipe_ptr = pipe;

	for (i = 0; i < NOFT && ofts[i].refCount > 0; i++); 	// Do the same for a second oft
	oft2 = &ofts[i];
	oft2->refCount = 1;
	oft2->mode = WRITE_PIPE;
	oft2->pipe_ptr = pipe;

	pipe->nreader = pipe->nwriter = 1;						// Set pipe's writer and reader count to 1

	for (i = 0; i < NFD; i += 2)
	{
		if (!running->fd[i] && !running->fd[i+1])			// If we can find two free fd's in a row
		{
			running->fd[i] = oft1;							// Set them to our two ofts
			running->fd[i+1] = oft2;
			break;
		}
	}

	put_word(i, running->uss, pd[0]);						// Write indices i and j to pd in user space
	put_word(i + 1, running->uss, pd[1]);

	printPipe(i);											// Print pipe for debugging purposes

	return 0;
}

int read_pipe(int fd, char *buf, int n)
{
	int r = 0;
	char c;
	PIPE *pipe;
	OFT *oft;

	if (n < 1)
		return 0;

	// validate fd. From fd, get OFT and pipe pointer p;
	if (fd < 0 || fd > NFD || !running->fd[fd])				// We can't write if fd is neg, higher than the amount we have, or if pipe hasn't been created
	{
		printf("write_pipe: illegal fd.\n");
		return -1;
	}

	oft = running->fd[fd];									// Get OFT from fd

	if (oft->mode != READ_PIPE)								// Check to see if pipe is open for reading
	{
		printf("write_pipe: pipe not open for writing.\n");
		return -1;
	}	

	printf("Pipe before read: \n");
	printPipe(fd);

	pipe = oft->pipe_ptr;									// Get pipe from OFT

	while (n)
	{
		while (pipe->data > 0 && n)							// While there's still data in the pipe and we still want more
		{
			c = pipe->buf[pipe->tail++];							// Grab a byte
			put_byte(c, running->uss, buf);					
			pipe->tail %= PSIZE;									// Wrap tail is passes end
			pipe->data--;									// Dec data, inc room
			pipe->room++;
			n--;
			r++;
			buf++;
		}

		if (r || !n)										// We read something or we don't want to read more
		{
			printf("Pipe after read: \n");
			printPipe(fd);
			kwakeup(&pipe->room);							// Wakeup any writers
			return r;
		}

		// If we didn't read anything
		if (pipe->nwriter)									// Check if pipe still has any writers
		{
			kwakeup(&pipe->room);							// Wakeup any writers
			ksleep(&pipe->data);							// sleep until writers write something new
			continue;										// When we wake back up, go try getting more data again
		}

		return 0;											// Pipe has no writer and no data
	}
}


int write_pipe(int fd, char *buf, int n)
{
	int r = 0;
	PIPE *pipe;
	OFT *oft;

	if (n < 1)
		return 0;

	// validate fd. From fd, get OFT and pipe pointer p;
	if (fd < 0 || fd > NFD || !running->fd[fd])				// We can't write if fd is neg, higher than the amount we have, or if pipe hasn't been created
	{
		printf("write_pipe: illegal fd.\n");
		return -1;
	}

	oft = running->fd[fd];									// Get OFT from fd

	if (oft->mode != WRITE_PIPE)							// Check to see if pipe is open for writing
	{
		printf("write_pipe: pipe not open for writing.\n");
		return -1;
	}	

	printf("Pipe before write: \n");
	printPipe(fd);

	pipe = oft->pipe_ptr;									// Get pipe from OFT

	while (n)
	{
		if (!pipe->nreader)									// No more readers
		{
			printf("write_pipe: pipe is broken.\n");
			kexit(BROKEN_PIPE);
		}

		while (pipe->room > 0 && n > 0)						// While there's still room in pipe and we want more written
		{
			pipe->buf[pipe->head++] = get_byte(running->uss, buf);	// Copy data from buf in user space into pipe buffer
			pipe->head %= PSIZE;									// Wrap head if passes end
			pipe->data++;									// Inc data, dec room 
			pipe->room--;
			n--;
			r++;
			buf++;
		}

		kwakeup(&pipe->data);								// Wakeup any readers

		if (n == 0)											// We wrote everything we wanted to
		{
			printf("Pipe after write: \n");
			printPipe(fd);
			return r;
		}

		ksleep(&pipe->room);								// Otherwise we have to sleep till reader reads and frees room
	}
}

int close_pipe(int fd)
{
	PIPE *pipe;
	OFT *oft;

	oft = running->fd[fd];								// Get the fd entry
	running->fd[fd] = 0;								// Clear it
	pipe = oft->pipe_ptr;								// Get the pipe

	oft->refCount--;									// Decrement the refCount no matter what

	if (oft->mode == WRITE_PIPE)						// If it was the write end
	{
		pipe->nwriter--;								// Dec nwriter

		if (oft->refCount == 1 && pipe->nreader < 1)	// If we're the last writer and there are no more readers
		{
			pipe->busy = 0;								// Free the pipe
			return 0;
		}

		kwakeup(&pipe->data);							// Otherwise wakeup readers so they can finish
		return 0;
	}

	if (oft->mode == READ_PIPE)							// If it was the read end
	{
		pipe->nreader--;								// Dec nreader

		if (oft->refCount == 1 && pipe->nwriter < 1)		// If we're the last reader and there are no more writers
		{
			pipe->busy = 0;								// Free the pipe
			return 0;
		}

		kwakeup(&pipe->room);							// Otherwise wakeup wrtiers so they can finish
		return 0;
	}

	return -1;											// Shouldn't happen
}

int pfd()
{
	int i, count = 0;
	char mode[5];

 	for (i = 0; i < NFD; i++)
    	if (running->fd[i]) count++;

 	printf("************** File Descriptors **************\n");

 	for (i = 0; i < NFD; i++)
 	{
   		if (running->fd[i])
   		{
   			switch(running->fd[i]->mode)
   			{
   				case READ_PIPE:
   					strcpy(mode, "READ ");
   					break;
   				case WRITE_PIPE:
   					strcpy(mode, "WRITE");
   					break;
   			}

      		printf("FD%d: %s | refCount = %d\n", i, mode, running->fd[i]->refCount);
   		}
 	}
 	printf("**********************************************\n");
}

int printPipe(int fd)
{
	PIPE *pipe;
	OFT *oft;
	int i = 0, j = 0;

	// Get the pipe
	oft = running->fd[fd];								// Get the fd entry
	pipe = oft->pipe_ptr;								// Get the pipe
	j = pipe->tail;

	printf("*********** fd %d's PIPE ***********\n", fd);
	printf("* Content: ");	

	while (i < pipe->data)								// Loop through data and print
	{
		printf("%c", pipe->buf[j]);

		j++; i++;
		j %= PSIZE;										// Wrap the index 
	}

	printf("\n* Data: %d\n", pipe->data);					// Print other pipe info
	printf("* Room: %d\n", pipe->room);	
	printf("* nreader: %d\n", pipe->nreader);	
	printf("* nwriter: %d\n", pipe->nwriter);	
	printf("***********************************\n");
}