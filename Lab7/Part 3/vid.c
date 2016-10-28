/*************************** Display Driver vid.c file of MTX Kernel **********************/
#define VDC_INDEX			0x3D4
#define VDC_DATA			0x3D5
#define CUR_SIZE			   10				//Cursor size register
#define	VID_ORG				   12				//Start address register
#define CURSOR				   14				//Cursor position register
#define LINE_WIDTH			   80				//# of characters on a line
#define SCR_LINES			   25				//# lines on the screen
#define SCR_BYTES			 4000				//Bytes of one screen=25*80
#define CURSOR_SHAPE		   15				//Block cursor for EGA/VGA
// attribute byte: 0x0HRGB, H=highlight, RGB determins color
u16 base = 0xB800;								//VRAM base address
u16 vid_mask = 0x3FFF;							//mask=Video RAM size -1.
u16 offset;										//Offset from VRAM segment base
int color;										//Attrubute byte
int org;										//Current display origin, r.e. VRAM base
int row, column;								//Logical row, col position

int vid_init()									//Initilize org=0 (row, column) = (0,0)
{
	int i, w;
	org = row = column = 0;						//Initilize globals
	color = 0x0A;								//High YELLOW;
	set_VDC(CUR_SIZE, CURSOR_SHAPE);				//Set cursor size
	set_VDC(VID_ORG, 0);						//Display origin to 0
	set_VDC(CURSOR, 0);							//Set cursor position to 0
	w = 0x0700;									//White, blank char
	for(i=0; i<25*80; i++)
	{
		put_word(w, base, 2*i);
	}
}

int scroll()									//Scroll UP one line
{
	u16 i, w, bytes;
	
	offset = org + SCR_BYTES + 2*LINE_WIDTH;
	if(offset <= vid_mask)						//Offset still within VRAM area
	{
		org += 2*LINE_WIDTH;
	}
	else										//Offset exceeds VRAM area. Reset to VRAM beginning
	{
		//Copy current rows 1-24 to BASE, then reset org to 0
		for(i=0; i<24*80; i++)
		{
			w = get_word(base, org + 160 + 2*i);
			put_word(w, base, 2*i);
		}
		org = 0;
	}
	// org has been set up properly
	offset = org + 2*24*80;						//offset = beginning of row 24
	//copy a line of blanks to row 24
	w = 0x0C00;									//HRGB = 1100 ==> Highlight RED, Null char
	for(i=0; i<80; i++)
	{
		put_word(w, base, offset + 2*i);
	}

	eraseTime();
	printTime();

	set_VDC(VID_ORG, org >> 1);					//Set VID_ORG to org
}

int move_cursor()								//Move cursor to current position
{
	int pos = 2*(row*80 + column);
	offset = (org + pos) & vid_mask;
	set_VDC(CURSOR, offset >> 1);

}

/*int updateTime(int _row, int _col)				//move cursor to clock position.
{
	int pos = 2*(_row*80, + _col);
	int _offset = (org + pos) & vid_mask;
	set_VDC(CURSOR, _offset >> 1);
}*/

//Display a char, handle special chars '\n', '\r', '\b', and now '\t'
int putc(char c)
{
	u16 w, pos;
	if (c == '\n')
	{
		row ++;
		if(row >= 25)							//At the end of the row
		{
			row = 24;
			scroll();
		}
		move_cursor();
		return;
	}
	if(c == '\r')
	{
		column = 0;
		move_cursor();							//move cursor to beginning of row
		return;
	}
	if(c == '\b')								//Delete a char
	{
		if (column > 0)
		{
			column--;
			move_cursor();
			put_word(0x0700, base, offset);
		}
		return;
	}
	/** TODO Implement '\t' **/

	// c is an ordinary character and we write it to screen
	pos = 2*(row * 80 + column);
	offset = (org + pos) & vid_mask;
	w = (color << 8) + c;						//This shrinks color down to 2-bits
	put_word(w, base, offset);
	column++;
	if(column >= 80)
	{
		column = 0;
		row++;
		if(row >= 25)
		{
			row = 24;
			scroll();
		}
	}
	move_cursor();
}

int set_VDC(u16 reg, u16 val)					//set VDC register reg to val
{
	lock();
	out_byte(VDC_INDEX, reg);					//Set index register
	out_byte(VDC_DATA, (val >> 8) & 0xFF);		//Output high byte
	out_byte(VDC_INDEX, reg + 1);				//Next index register
	out_byte(VDC_DATA, val & 0xFF);				//Output low byte
	unlock();
}