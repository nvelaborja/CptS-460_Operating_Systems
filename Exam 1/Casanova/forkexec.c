
int fork()
{
  int pid; u16 segment;
  PROC *p = kfork(0);               // kfork() a new child without loading an image

  //printf("P%d Enter fork \n", running->pid);

  if (!p) return -1;                // kfork failed

  segment = (p->pid + 1) * IMGSIZE; // New child's segment
  copyImage (running->uss, segment, 32 * 1024);   // Copy 32K words

  p->uss = segment;                 // Update childs uss to it's new segment
  p->usp = running->usp;            // Same as parent's usp

  // Change uDS, uES, uCS, and AX in child's ustack
  put_word(segment, segment, p->usp);             // uDS = segment
  put_word(segment, segment, p->usp + 2);         // uES = segment
  put_word(0,       segment, p->usp + 2 * 8);     // uAX = 0
  put_word(segment, segment, p->usp + 2 * 10);    // uCS = segment

  return p->pid;
}

int exec(char *fileName)
{
  int i, j, length = 0, offset;
  char file[64], string[64], *cp = string, *location = "/bin/";
  u16 segment = IMGSIZE * (running->pid + 1);

  // Put /bin/ at the beginning of file name since all executables will be in there
  for (i = 0; i < 5; i++)
    file[i] = location[i];
  file[5] = 0;

  printf("File: %s\n", file);
  
  i = 0;

  // Get command from U space with a length limit of 64
  while ((*cp++ = get_byte(running->uss, fileName++)) && length++ < 64);

  // Tokenize on the first space to get filename
  while (string[i] && string[i] != ' ')
  {
    file[i + 5] = string[i];          // Add after the /bin/
    i++;
  }
  file[i + 5] = 0;    
  length = strlen(string); 

  printf("P%d Enter exec, string=%s, file=%s\n", running->pid, string, file);

  if (load(file, segment) < 0)        // Load file into segment
    return -1;                      // return -1 to umode on failure

  if (length % 2 == 1) length++;         // Must have an even length string
  else length += 2;

  // Re-initialize process ustack for it return to VA = 0
  for (i = 1; i <= 12; i++)
    put_word(0, segment, segment - 2*i - length - 2);

  running->usp = segment - 24 - length - 2;
  running->uss = segment;

  put_word(segment - length, segment, segment - 2 - length);

  // Move the rest of the string (if any) to top of ustack in user space
  for (i = 0; i < length; i++)
    put_word(string[i], segment, segment + i - length);

 

  put_word(segment, segment, segment - 2*12 - length - 2);  // uDS = segment
  put_word(segment, segment, segment - 2*11 - length - 2);  // uES = segment
  put_word(segment, segment, segment - 2*2 - length - 2);   // uCS = segment, uPC = 0
  put_word(0x0200, segment, segment - 2*1 - length - 2);    // Umode flag
}

int copyImage (u16 pseg, u16 cseg, u16 size)
{
  u16 i;

  //printf("P%d Enter copyImage \n", running->pid);

  for (i = 0; i < size; i++)
    put_word (get_word(pseg, i * 2), cseg, i *2);
}
