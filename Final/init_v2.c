//*************************************************************************
//                      Logic of init.c 
// NOTE: this init.c creates only ONE login process on console=/dev/tty0
// YOUR init.c must also create login processes on serial ports /dev/ttyS0
// and /dev/ttyS1.. 
//************************************************************************

// TODO: 
//  x  Create login processes on serial ports /dev/ttyS0 and /dev/ttyS1

int pid, child, status, children[3], i;

#include "ucode.c"  //<========== AS POSTED on class website
#include "user.h"

main(int argc, char *argv[])
{
  // open /dev/tty0 as 0 (READ) and 1 (WRTIE) in order to display messages
  open("dev/tty0", O_RDONLY);
  open("dev/tty0", O_WRONLY);

  // Now we can use printf, which calls putc(), which writes to stdout
  printf("VBINIT: fork a login task on console\n"); 
 
  for (i = 0; i < 3; i++)
  {
    child = i;
    children[i] = fork();

    if (!children[i])
      login();
  }

  parent();
}       

int login()
{
  switch (i)
  {
    case 0:
      exec("login /dev/ttyS0");
      break;
    case 1:
      exec("login /dev/ttyS1");
      break;
    case 2:
      exec("login /dev/tty0");
      break;
    default:
      printf("VBINIT: login error.\n");
      break;
  }

  return FN_SUCCESS;
}
      
int parent()
{
  int mychild = 0;
  int replace = 0;

  while (1)
  {
    pid = wait(&status);

    for(i = 0; i < 3; i++)
    {
      if (pid == children[i])
      {
        mychild++;
        replace = i;
      }
    }

    if (mychild)
    {
      i = replace;
      children[replace] = fork();

      if (!children[replace])
        login();
    }
  }

  return FN_SUCCESS;
}