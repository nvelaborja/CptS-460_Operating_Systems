/*************** type.h file *****************/

#define NPROC 9
#define SSIZE 1024

/******* PROC status ********/
#define FREE 0
#define READY 1
#define RUNNING 2				// for clarity only, not needed or used
#define STOPPED 3
#define SLEEP 4
#define ZOMBIE 5

typedef struct proc{
    struct proc *next;
    int    *ksp;
    int    pid;                	// add pid for identify the proc
    int    status;             	// status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               	// parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;
    int    kstack[SSIZE];      	// per proc stack area
}PROC;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
