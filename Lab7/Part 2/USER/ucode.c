// ucode.c for EXAM 1 PIPE
// Nathan VelaBorja - 11392441

extern int main(int argc, char *argv[]);

char *cmd[] = { "getpid", "ps", "chname", "kfork", "switch", "wait", "exit", "kgetc", "kputc", "fork", "exec", "pipe", "pread", "pwrite", "pclose", "pfd", "itimer", 0 };
int pd[2];      // For pipe usage

#define LEN 64
#define NUM_CMD 17

int show_menu()
{
   printf("*********************** Menu **********************\n");
   printf("*   ps     chname       kfork      switch    wait *\n");
   printf("*  exit     kgetc       kputc       fork     exec *\n");
   printf("*  pipe     pread      pwrite      pclose    pfd  *\n");
   printf("*                      itimer                     *\n");
   printf("***************************************************\n");
}

int find_cmd(char *name)
{
  // return command index
  int i = 0;

  //printf("P Enter find_cmd \n");

  while (i < NUM_CMD)
  {
    if (streq(name, cmd[i]))
      return i;
    i++;
  }

  return -1;
}

int getpid()
{
  //printf("P Enter getpid \n");
   return syscall(0,0,0);
}

int ps()
{
  //printf("P Enter ps \n");
   return syscall(1, 0, 0);
}

int chname()
{
    char s[32];
    //printf("P Enter chname \n");
    printf("input new name : ");
    gets(s);
    return syscall(2, s, 0);
}

int kfork()
{   
  int child, pid;
  char fileName[64];
  printf("Input a filename for kfork: ");
  gets(fileName);

  printf("proc %d enter kernel to kfork a child\n", getpid()); 
  child = syscall(3, fileName, 0);
  printf("proc %d kforked a child %d\n", pid, child);
}    

int kswitch()
{
  //printf("P Enter kswitch \n");
  return syscall(4,0,0);
}

int wait()
{
    int child, exitValue;
    //printf("P Enter wait \n");
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n"); 
} 

int atoi(char *s)
{
  int v = 0;
  while(*s){
    v = v*10 + (*s-'0');
    s++;
  }
  return v;
}

int geti()
{
  char s[16];
  gets(s);
  return atoi(s);
}

int exit()
{
  int exitValue;
  //printf("P Enter exit \n");
  printf("enter an exitValue: ");
  exitValue = geti();
  printf("exitvalue=%d\n", exitValue);
  printf("enter kernel to die with exitValue=%d\n", exitValue);
  _exit(exitValue);
}

int _exit(int exitValue)
{
  //printf("P Enter _exit \n");
  return syscall(6,exitValue,0);
}

int invalid(char *name)
{
  printf("Invalid command : %s\n", name);
}

int streq(char *string1, char *string2)
{
  int i = 0;
  //printf("P Enter streq \n");

  while (string1[i] != 0)           // Loop through and check strings
  {
    if (string1[i] != string2[i])   // Return false if any chars don't match
      return 0;
    i++;
  }

  if (string2[i] != 0)              // Make sure string2 also ends at the same index
    return 0;

  return 1;
}

char getc()
{
  return syscall(7, 0, 0);
}

int putc(char c)
{
  return syscall(8, c, 0);
}

int fork()
{
  return syscall(9, 0, 0);
}

int exec()
{
  char s[64];

  printf("Input a command to exec: ");
  gets(s);
  return syscall(10, s, 0);
}

int pipe()
{
  //printf("Enter pipe\n");

  return syscall(11, pd, 0);
}

int read_pipe()
{
  char buffer[1024], response[32];
  int bytesRead = 0, bytesRequested = 0;
  int fd = 0;

  //printf("Enter read_pipe\n");

  pfd();

  printf("Enter fd for read: ");                            // Prompt for read location
  gets(response);
  fd = atoi(response);

  printf("Enter number of bytes would you like to read: "); // Prompt for nbytes
  gets(response);
  bytesRequested = atoi(response);

  bytesRead = syscall(12, fd, buffer, bytesRequested);      // Go to kernel to read pipe

  if (bytesRead)
  {
    printf("Read %d bytes from fd %d:\n", bytesRead, fd);
    buffer[bytesRead] = 0;                                  // Terminate the string so we can print it
    printf("  %s\n", buffer);

    return 1;
  }

  printf("Couldn't read anything from pipe.\n");
  return -1;
}

int write_pipe()
{
  char response[32];
  int bytesWritten = 0, bytesRequested = 0;
  int fd = 0;

  //printf("Enter write_pipe\n");

  pfd();

  printf("Enter fd for write: ");                           // Prompt for fd
  gets(response);
  fd = atoi(response);

  printf("Enter what you want to write: ");                 // Prompt for data to write
  gets(response);
  bytesRequested = strlen(response);

  bytesWritten = syscall(13, fd, response, bytesRequested);   // Go to kernel to write to pipe

  if (bytesWritten)
  {
    printf("Wrote %d bytes to fd %d:\n", bytesWritten, fd);
    return 1;
  }

  printf("Couldn't write anything to the pipe.\n");
  return -1;
}

int close_pipe()
{
  char response[32];
  int fd = 0;

  //printf("Enter close_pipe\n");

  printf("Enter fd you would like to close: ");
  gets(response);
  fd = atoi(response);

  return syscall(14, fd, 0);
}

int pfd()
{
  //printf("Enter pfd\n");
  return syscall(15, 0, 0);
}

int printArgv(char *argv[])
{
  int i = 0;

  printf("\n***** argv[] contents *****\n");

  while (argv[i] && i < 32)
  {
    printf("Argv[%d]: %s\n", i, argv[i]);
    i++;
  }

  if (i == 0)
    printf("    argv[] is empty @.@\n");

  printf("***************************\n");
}

int itimer()
{
  char response[8];
  int time;
    
  //printf("P Enter itimer \n");

  printf("How long? : ");
  gets(response);
  time = atoi(response);

  printf("\n\nTIME=%d\n\n", time);

  return syscall(16, time, 0);
}