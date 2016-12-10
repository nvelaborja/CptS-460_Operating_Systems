//***********************************************************************
//                   LOGIC of login.c file
//***********************************************************************

#include "user.h"

char *tty;
int stdin, stdout, uid, gid;
char homeDir[INPUTLEN], uProgram[INPUTLEN], uname[INPUTLEN];

#include "ucode.c"

main(int argc, char *argv[])   // invoked by exec("login /dev/ttyxx")
{
  char username[INPUTLEN], password[INPUTLEN];

  tty = argv[1];

  close(0); close(1); //close(2); // login process may run on different terms

  // open its own tty as stdin, stdout, stderr
  stdin  = open(tty, O_RDONLY);
  stdout = open(tty, O_WRONLY);

  settty(tty);   // store tty string in PROC.tty[] for putc()

  // NOW we can use printf, which calls putc() to our tty
  printf("VBLOGIN : open %s as stdin, stdout\n", tty);

  signal(2,1);  // ignore Control-C interrupts so that 
                // Control-C KILLs other procs on this tty but not the main sh

  while(1){

    printf("Username: ");
    gets(username);
    printf("Password: ");
    gets(password);

    if (ValidateUser(username, password) == TRUE)     // verify user name and passwd from /etc/passwd file
    {
      printf("Welcome %s!\n", username);
      chuid(uid, gid);                                // setuid to user uid.
      chdir(homeDir);                                 // chdir to user HOME directory.
      exec(uProgram);                                 // exec to the program in users's account

      return FN_SUCCESS;
    }

    printf("login failed, try again\n");
  }

  exit(EXIT_SUCCESS);
}

int ValidateUser(char *username, char *password)
{
  int pfile = open("etc/passwd", O_RDONLY);
  int isUser = FALSE;
  int isPassword = FALSE;
  char *token = 0;
  char *delimiter = ":\n";
  char buffer[BUFLEN];

  if (pfile == 0)
    return FN_FAILURE;

  read(pfile, buffer, BUFLEN);                        // Read whole file

  token = strtok(buffer, delimiter);                  // Tokenize

  while (token != 0)
  {
    if (strcmp(token, username) == 0)                 // Check for user match
      isUser = TRUE;
    
    if (isUser)                                       // If username found, next token is that users password
    {
      token = strtok(0, delimiter);               // Get password

      if (strcmp(token, password) == 0)               // If match, get info in next tokens
      {
        uid = atoi(strtok(0, delimiter));          // get uid
        gid = atoi(strtok(0, delimiter));          // get gid
        strcpy(uname, strtok(0, delimiter));       // get name
        strcat(homeDir, strtok(0, delimiter));     // get home dir

        if (DEBUGLVL >= 1)
          printf("Home Directory for user '%s': %s\n", username, homeDir);

        strcpy(uProgram, strtok(0, delimiter));    // get user program

        close(pfile);
        return TRUE;
      }
    }

    token = strtok(0, delimiter);                  // get next token
  }

  close(pfile);
  return FALSE;
}