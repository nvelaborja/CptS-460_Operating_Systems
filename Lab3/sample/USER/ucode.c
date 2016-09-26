// ucode.c file

char *cmd[] = { "getpid", "ps", "chname", "kfork", "switch", "wait", "exit", 0 };

#define LEN 64
#define NUM_CMD 7

int show_menu()
{
   printf("******************* Menu *******************\n");
   printf("*  ps  chname  kfork   switch  wait  exit  *\n");
   printf("********************************************\n");
}

int find_cmd(char *name)
{
  // return command index
  int i = 0;

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
   return syscall(0,0,0);
}

int ps()
{
   return syscall(1, 0, 0);
}

int chname()
{
    char s[32];
    printf("input new name : ");
    gets(s);
    return syscall(2, s, 0);
}

int kfork()
{   
  int child, pid;
  pid = getpid();
  printf("proc %d enter kernel to kfork a child\n", pid); 
  child = syscall(3, 0, 0);
  printf("proc %d kforked a child %d\n", pid, child);
}    

int kswitch()
{
  return syscall(4,0,0);
}

int wait()
{
    int child, exitValue;
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
  printf("enter an exitValue: ");
  exitValue = geti();
  printf("exitvalue=%d\n", exitValue);
  printf("enter kernel to die with exitValue=%d\n", exitValue);
  _exit(exitValue);
}

int _exit(int exitValue)
{
  return syscall(6,exitValue,0);
}

int invalid(char *name)
{
  printf("Invalid command : %s\n", name);
}

int streq(char *string1, char *string2)
{
  int i = 0;

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