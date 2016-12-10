/*******************************************************\
					Nathan VelaBorja
				    November 30, 2016
						user.h
\*******************************************************/

//------------ Bools --------------
#define FALSE 0
#define TRUE 1
//---------------------------------

//----------- User IO -------------
#define INPUTLEN 64
#define BUFLEN 128
#define MAX_PATH 128
//---------------------------------

//---------- Exit Status ----------
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
//---------------------------------

//-------- Function Status --------
#define FN_FAILURE 0
#define FN_SUCCESS 1
//---------------------------------

//---------- DEBUG mode -----------
#define DEBUGLVL 0		// 0 - none, 1 - simple, 2 - verbose
//---------------------------------

//---------- Screen Info ----------
#define SCREEN_W 80
#define SCREEN_H 20
//---------------------------------

//---------- File Desc ------------
#define ENDFILE -1
//---------------------------------

//---------- Prototypes -----------

int login(int process);
int parent();

int ValidateUser(char *username, char *password);

//---------------------------------