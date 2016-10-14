// type.h for EXAM 1 PIPE
// Nathan VelaBorja - 11392441

#define NPROC   9
#define SSIZE   1024
#define NAMELEN 32
#define IMGSIZE 0x1000

/******* PROC status ********/
#define FREE    0
#define READY   1
#define RUNNING 2				// for clarity only, not needed or used
#define STOPPED 3
#define SLEEP   4
#define ZOMBIE  5

/******* Pipe Info **********/
#define PSIZE       10
#define NPIPE       10
#define READ_PIPE    4
#define WRITE_PIPE   5
#define NOFT        20
#define NFD         10
#define BROKEN_PIPE 99


typedef struct proc{
    struct proc     *next;          
    int             *ksp;                // Offset 2
    int             uss, usp;            // Offset 4, 6
    int             inkmode;             // Offset 8
    int             pid;                	// add pid for identify the proc
    int             status;             	// status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int             ppid;               	// parent pid
    struct proc     *parent;
    int             priority;
    int             event;
    int             exitCode;
    char            name[NAMELEN];       // String name for PROC
    struct oft      *fd[NFD];
    int             kstack[SSIZE];      	// per proc stack area
}PROC;

typedef struct pipe{
    char buf[PSIZE];            // circular data buffer
    int head, tail;             // circular buf indices 
    int data, room;             // number of data & room in pipe
    int nreader, nwriter;       // number of readers, writers on pipe
    int busy;                   // Flag to show if pipe is free or not
}PIPE;

typedef struct oft{
    int mode;                   // READ_PIPE / WRITE_PIPE
    int refCount;               // number of PROCs using this pipe
    struct pipe *pipe_ptr;      // Pointer to the PIPE object
}OFT;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
