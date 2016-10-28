// u1.c for EXAM 1 PIPE
// Nathan VelaBorja - 11392441

#include "ucode.c"
int color;

main(int argc, char *argv[])
{ 
  char name[64], c = '\0'; int pid, cmd;
  int i;

  //printf("Enter main \n");

  while(1){

    pid = getpid();
    color = getpid() % 7 + 1;
       
    printf("----------------------------------------------\n");
    printf("I am proc %d in U mode: running segment=%x\n\n",getpid(), getcs());
    //printArgv(argv);
    show_menu();
    printf("Command ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
      case 0 : getpid();    break;
      case 1 : ps();        break;
      case 2 : chname();    break;
      case 3 : kmode();     break;
      case 4 : kswitch();   break;
      case 5 : wait();      break;
      case 6 : exit();      break;
      case 7 : c = getc();  break; // Not really sure what to do here, but this will show it works
      case 8 : putc(c);     break;
      case 9 : fork();      break;
      case 10: exec();      break;
      case 11: pipe();      break;
      case 12: read_pipe(); break;
      case 13: write_pipe();break;
      case 14: close_pipe();break;
      case 15: pfd();       break;
      case 16: itimer();    break;

      default: invalid(name); break;
    }
  }
}


