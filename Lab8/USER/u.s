        .globl _main0,_syscall,_exit,_getcs,auto_start
        .globl _getc

auto_start:
        call    _main0

! if main() ever return, exit(0)
	push   #0
        call    _exit

_syscall:
        int    80
        ret


	
_getcs:
        mov   ax, cs
        ret

        !---------------------------------------------
        !  char getc( ): call BIOS to get a char
        !---------------------------------------------
_getc:
            xorb   ah,ah           ! clear ah
            int    0x16            ! call BIOS to get a char in AX
            andb   al,#0x7F        ! 7-bit ascii  
            ret 
	
