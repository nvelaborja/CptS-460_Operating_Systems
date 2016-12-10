#include "ucode.c"

main(int argc, char *argv[ ])
{
  int i;
  printf("this is a test program. Nathan!!\n");

  printf("argc=%d\n", argc);

  for (i=0; i<argc; i++){
    printf("argv[%d]=%s\n", i, argv[i]);
  }

  printf("the end\n");
}

