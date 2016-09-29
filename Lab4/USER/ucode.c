// ucode.c file

extern int main(int argc, char *argv[]);

char *cmd[] = { "getpid", "ps", "chname", "kfork", "switch", "wait", "exit", "kgetc", "kputc", "fork", "exec", 0 };

#define LEN 64
#define NUM_CMD 11

int show_menu()
{
   printf("***************** Menu ******************\n");
   printf("*   ps   chname  kfork  switch   wait   *\n");
   printf("*  exit  kgetc   kputc   fork    exec   *\n");
   printf("*****************************************\n");
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

char kgetc()
{
  return syscall(7, 0, 0);
}

int kputc(char c)
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