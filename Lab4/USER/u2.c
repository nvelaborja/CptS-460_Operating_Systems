#include "ucode.c"
int color;

main()
{ 
  char name[64], c = '\0'; int pid, cmd;

  //printf("Enter main \n");

  while(1){
    pid = getpid();
    color = 0x0C;
       
    printf("----------------------------------------------\n");
    printf("Yo soy proc %d en U modo: segmento de corriente=%x\n\n",getpid(), getcs());
    show_menu();
    printf("Comando ? ");
    gets(name); 
    if (name[0]==0) 
        continue;

    cmd = find_cmd(name);
    switch(cmd){
      case 0 : getpid();   break;
      case 1 : ps();       break;
      case 2 : chname();   break;
      case 3 : kfork();    break;
      case 4 : kswitch();  break;
      case 5 : wait();     break;
      case 6 : exit();     break;
      case 7 : c = kgetc(); break; // Not really sure what to do here, but this will show it works
      case 8 : kputc(c);   break;
      case 9 : fork();     break;
      case 10: exec();     break;

      default: invalid(name); break;
    }
  }
}



