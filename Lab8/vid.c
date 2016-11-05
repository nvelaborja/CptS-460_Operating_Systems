/*********** Display Driver vid.c file of MTX kernel ************/
#define VDC_INDEX		0x3D4
#define VDC_DATA 		0x3D5
#define CUR_SIZE  		10 		// cursor size register
#define VID_ORG  		12 		// start address register
#define CURSOR      	14 		// cursor position register
#define LINE_WIDTH  	80 		// # characters on a line
#define SCR_LINES   	25 		// # lines on the screen
#define SCR_BYTES   	4000 	// bytes of one screen=25*80
#define CURSOR_SHAPE 	15 		// block cursor for EGA/VGA
// attribute byte: 0x0HRGB, H=highLight; RGB determine color
u16 base = 0xB800;				// VRAM base address
u16 vid_mask = 0x3FFF;			// mask=Video RAM size - 1
u16 offset;						// offset from VRAM segment base
int color;						// attribute byte
int org;						// current display origin, r.e. VRAM base
int row, column;				// logical row, col position

int vid_init() // initializes org=0 (row,column)=(0,0)
{ 
	int i, w;
	org = row = column = 0; 	// initialize globals
	color = 0x0A;				// high YELLOW;
	set_VDC(CUR_SIZE, CURSOR_SHAPE); // set cursor size
	set_VDC(VID_ORG, 0);		// display origin to 0
	set_VDC(CURSOR, 0);			// set cursor position to 0
	w = 0x0700;					// White, blank char
	for (i=0; i<25*80; i++)		// clear screen
		put_word(w, base, 0+2*i);	// write 25*80 blanks to VRAM
}

int scroll() //scroll UP one line
{ 
	u16 i, w, bytes;
	// test offset = org + ONE screen + ONE more line

	offset = org + SCR_BYTES + 2*LINE_WIDTH;
	if (offset <= vid_mask)		// offset still within VRAM area
		org += 2*LINE_WIDTH;	// just advance org by ONE line
	else{ 						// offset exceeds VRAM area ==> reset to VRAM beginning by
								// copy current rows 1-24 to BASE, then reset org to 0
		for (i=0; i<24*80; i++){
			w = get_word(base, org+160+2*i);
			put_word(w, base, 0+2*i);
		}
		org = 0;
	}
	// org has been set up properly
	offset = org + 2*24*80;		// offset = beginning of row 24
	// copy a line of BLANKs to row 24
	w = 0x0C00; // HRGB=1100 ==> HighLight RED, Null char
	for (i=0; i<80; i++)
		put_word(w, base, offset + 2*i);
	set_VDC(VID_ORG, org >> 1);	// set VID_ORG to org

	// from timer.c
	clear_clock();
	wall_clock();
}

int move_cursor() // move cursor to current position
{
	int pos = 2*(row*80 + column);
	offset = (org + pos) & vid_mask;
	set_VDC(CURSOR, offset >> 1);
}
// display a char, handle special chars '\n','\r','\b'

int putc(char c)
{ 
	u16 w, pos;
	if (c=='\n'){
		row++;
		if (row>=25){
			row = 24;
			scroll();
		}
		move_cursor();
		return;
	}

	if (c=='\r'){
		column=0;
		move_cursor();
		return;
	}

	if (c=='\b'){
		if (column > 0){
			column--;
			move_cursor();
			put_word(0x0700, base, offset);
		}
		return;
	}

	// c is an ordinary char
	pos = 2*(row*80 + column);
	offset = (org + pos) & vid_mask;
	w = (color << 8) + c;
	put_word(w, base, offset);
	column++;
	if (column >= 80){
		column = 0;
		row++;
		if (row>=25){
			row = 24;
			scroll();
		}
	}
	move_cursor();
}

int set_VDC(u16 reg, u16 val) //set VDC register reg to val
{
	lock();
	out_byte(VDC_INDEX, reg);			// set index register
	out_byte(VDC_DATA, (val>>8)&0xFF);  // output high byte
	out_byte(VDC_INDEX, reg + 1);		// next index register
	out_byte(VDC_DATA, val&0xFF);		// output low byte
	unlock();
}