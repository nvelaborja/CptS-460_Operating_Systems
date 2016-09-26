/*************** type.h file *****************/

#define NPROC 9
#define SSIZE 1024
#define FREE 0
#define READY 1
#define RUNNING 2		// for clarity only, not needed or used
#define STOPPED 3
#define SLEEP 4
#define ZOMBIE 5

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;

typedef struct proc
{
	struct proc *next;
	struct proc *parent; 	// pointer to parent PROC
	int *ksp;
	int pid; 		// process ID number
	int ppid; 		// parent pid		
	int status; 		// status = FREE|READY|RUNNING|SLEEP|ZOMBIE
	int priority;
	int event; 		// sleep event
	int exitCode; 		// exit code
	int kstack[SSIZE];
}PROC;

