int argc;
char *argv[32], *cp;

int main0(char *s)
{
  //char *string = "This is a test of the tokenize function";
  //printf("Enter main0, string=%s \n", s);
  if (tokenize(s, argv, &argc, ' '))
  {
    printArgv();
    main(argc, argv);
  }

  return 0;
}

/// Tokenize path and put tokens into argv, and token count in argc
int tokenize(char *path, char *argv[], int *argc, char delimiter)
{
  int i, nnames = 0;

  //printf("Enter tokenize \n");
  
  cp = strtok(path, " ");

  while (cp != 0)
  {
    argv[nnames] = cp;
    nnames++;
    cp = strtok(0, " ");
  }

  *argc = nnames;

  if(nnames) return 1;
  return -1;
}

int printArgv()
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

  printf("       argc = %d         \n", argc);

  printf("***************************\n");
}