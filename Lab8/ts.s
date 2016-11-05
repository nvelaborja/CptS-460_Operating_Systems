                BOOTSEG = 0x1000
                SSP     = 0
!               IMPORTS and EXPORTS
                .globl _main,_prints                                 
                .globl _tswitch,_running,_scheduler
                .globl _proc, _procSize
                .globl _int80h,_kcinth
                .globl _getcs, _goUmode, _procSize
                .globl _lock, _unlock, _int_off, _int_on
                .globl _in_byte,_out_byte

                .globl _tinth, _thandler
                !.globl _kbinth, _kbhandler
                  
                .globl _s0inth, _s0handler
                .globl _s1inth, _s1handler

                jmpi   start,BOOTSEG    ! just to make sure CS=0x1000

start:  mov  ax,cs
  mov  ds,ax
  mov  ss,ax
  mov  es,ax
  mov  sp,#_proc
  add  sp,_procSize      ! set stack pointer to high end of PROC[0] 

  mov     ax,#0x0003     ! these will start in MONO display mode
  int     #0x10          ! needed by vid driver later
    
  call _main

  ! if ever return, just hang     
        mov   ax, #msg
        push  ax
        call  _prints
!_exit:
dead:   jmp   dead
msg:    .asciz "BACK TO ASSEMBLY AND HANG\n\r"    

_kreboot:
        jmpi 0,0xFFFF 
!*************************************************************
!     KCW  added functions for MT system
!************************************************************
_tswitch:
SAVE: 
    cli
  push ax
  push bx
  push cx
  push dx
  push bp
  push si
  push di
  pushf
  mov  bx,_running
  mov  2[bx],sp

FIND: call _scheduler

RESUME:
  mov  bx,_running
  mov  sp,2[bx]
  popf
  pop  di
  pop  si
  pop  bp
  pop  dx
  pop  cx
  pop  bx
  pop  ax

  sti
  ret

_getcs: mov   ax, cs
        ret

  !These offsets are defined in struct proc
USS =   4
USP =   6
INK =   8
  
! as86 macro: parameters are ?1 ?2, etc 
! as86 -m -l listing src (generates listing with macro expansion)

         MACRO INTH
          push ax
          push bx
          push cx
          push dx
          push bp
          push si
          push di
          push es
          push ds

          push cs
          pop  ds

          mov bx,_running     ! ready to access proc
          inc INK[bx]
          cmp INK[bx],#1
          jg   ?1

          ! was in Umode: save interrupted (SS,SP) into proc
          mov USS[bx],ss        ! save SS  in proc.USS
          mov USP[bx],sp        ! save SP  in proc.USP

          ! change DS,ES,SS to Kernel segment
          mov  di,ds            ! stupid !!        
          mov  es,di            ! CS=DS=SS=ES in Kmode
          mov  ss,di

          mov  sp,_running      ! sp -> runnings kstack[] high end
          add  sp,_procSize

?1:       call  _?1             ! call handler in C

          br    _ireturn        ! return to interrupted point

         MEND


_int80h: INTH kcinth
_tinth:  INTH thandler
_s0inth: INTH s0handler
_s1inth: INTH s1handler


!*===========================================================================*
!*    _ireturn  and  goUmode()                   *
!*===========================================================================*
! ustack contains    flag,ucs,upc, ax,bx,cx,dx,bp,si,di,es,ds
! uSS and uSP are in proc
_ireturn:
_goUmode:
        cli
        mov bx,_running   ! bx -> proc
        dec INK[bx]       
        cmp INK[bx],#0
        jg  xkmode        ! if proc not in in umode 

! restore uSS, uSP from running PROC ! in umode
        mov ax,USS[bx]
        mov ss,ax               ! restore SS
        mov sp,USP[bx]          ! restore SP
xkmode:                         
        pop ds
        pop es
        pop di
        pop si
        pop bp
        pop dx
        pop cx
        pop bx
        pop ax 
        iret

_int_off:             ! cli, return old flag register
        pushf
        cli
        pop ax
        ret

_int_on:              ! int_on(int SR)
        push bp
        mov  bp,sp
        mov  ax,4[bp] ! get SR passed in
        push ax
        popf
        pop  bp
        ret

!*===========================================================================*
!*        lock               *
!*===========================================================================*
_lock:  
  cli     ! disable interrupts
  ret     ! return to caller

!*===========================================================================*
!*        unlock               *
!*===========================================================================*
_unlock:
  sti     ! enable interrupts
  ret     ! return to caller

!*===========================================================================*
!*        in_byte              *
!*===========================================================================*
! PUBLIC unsigned in_byte[port_t port];
! Read an [unsigned] byte from the i/o port  port  and return it.

_in_byte:
        push    bp
        mov     bp,sp
        mov     dx,4[bp]
  inb     al,dx   ! input 1 byte
  subb  ah,ah   ! unsign extend
        pop     bp
        ret

!*===========================================================================*
!*        out_byte             *
!*==============================================================
! out_byte[port_t port, int value];
! Write  value  [cast to a byte]  to the I/O port  port.

_out_byte:
        push    bp
        mov     bp,sp
        mov     dx,4[bp]
        mov     ax,6[bp]
  outb  dx,al     ! output 1 byte
        pop     bp
        ret
