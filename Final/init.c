//*************************************************************************
//                      Logic of init.c 
// NOTE: this init.c creates only ONE login process on console=/dev/tty0
// YOUR init.c must also create login processes on serial ports /dev/ttyS0
// and /dev/ttyS1.. 
//************************************************************************

// TODO: 
//  x  Create login processes on serial ports /dev/ttyS0 and /dev/ttyS1

int pid, tty0, ttyS0, ttyS1, status;
int stdin, stdout, stderr;

#include "ucode.c"  //<========== AS POSTED on class website
#include "user.h"

main(int argc, char *argv[])
{
  // open /dev/tty0 as 0 (READ) and 1 (WRTIE) in order to display messages
  stdin = open("dev/tty0", O_RDONLY);
  stdout = open("dev/tty0", O_WRONLY);
  stderr = open("dev/tty0", O_RDWR);

  // Now we can use printf, which calls putc(), which writes to stdout
  printf("VBINIT: fork a login task on console\n"); 
  tty0 = fork();
  printf("tty0:%d\n", tty0);

  if (tty0)
  {
    ttyS0 = fork();

    if (ttyS0)
    {
      ttyS1 = fork();

      if (ttyS1)
      {
        parent();
        return FN_SUCCESS;
      }

      login(2);
      return FN_SUCCESS;
    }

    login(1);
    return FN_SUCCESS;
  }

  login(0);
  return FN_SUCCESS;
}       

int login(int process)
{
  switch (process)
  {
    case 0:
      exec("login /dev/tty0");
      break;
    case 1:
      exec("login /dev/ttyS0");
      break;
    case 2:
      exec("login /dev/ttyS1");
      break;
    default:
      printf("VBINIT: login error.\n");
      break;
  }
  exec("login /dev/tty0");
  return FN_SUCCESS;
}
      
int parent()
{
  while(1){
    printf("VBINIT: waiting .....\n");

    pid = wait(&status);

    if (pid == tty0)
    {
      tty0 = fork();

      if (tty0 == 0)
        login(0);
    }
    else if (pid == ttyS0) 
    {
      ttyS0 = fork();

      if (ttyS0 == 0)
        login(1);
    }
    else if (pid == ttyS1)
    {
      ttyS1 = fork();

      if (ttyS1 == 0)
        login(2);
    }
    else printf("VBINIT: buried an orphan child %d\n", pid);
  }

  return FN_SUCCESS;
}