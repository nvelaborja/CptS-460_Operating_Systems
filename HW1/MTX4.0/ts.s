# -------------- ts.s file --------------------------- 
          MTXSEG = 0x1000
	
         .globl  _main,_proc,_procSize,_running,_scheduler
         .global  _tswitch

	
          jmpi  start, MTXSEG
start:    mov   ax,cs
          mov   ds,ax
          mov   ss,ax
	
          mov   sp,#_proc       ! sp->proc
          add   sp, _procSize   ! sp->high end of proc
	
          call  _main

dead:	  jmp   dead
	
_tswitch:
SAVE:	  push  ax
          push  bx
          push  cx
          push  dx
          push  bp
          push  si
          push  di
	  pushf
	  mov   bx,_running
 	  mov   2[bx], sp

NEXT_RUN: call	_scheduler

_resume:
RESUME:   mov	bx,_running
	  mov	sp, 2[bx]
	  popf
	  pop   di
          pop   si
          pop   bp
          pop   dx
          pop   cx
          pop   bx
          pop   ax

	  ret
