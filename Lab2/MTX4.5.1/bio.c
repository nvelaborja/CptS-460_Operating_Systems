char *table = "0123456789ABCDEF";

// print string
void prints(char *s)	
{
	while (*s != '\0')
	{
		putc(*s);
		s++;
	}
}

// recursively print value
int rpu(u16 x, int BASE) 
{
	char c;
	
	if (x)
	{
		c = table[x % BASE];
		rpu(x / BASE, BASE);
		putc(c);
	}
}

// print unsigned int
int printu(u16 x) 	
{
	if (x==0)
		putc('0');
	else
		rpu(x, 10);
	putc(' ');
}

// print signed int
int printd(int x)	
{
	if (x < 0)
	{
		putc('-');
		x = -x;
	}
	rpu(x, 10);
	//putc(' ');
}

// print octal
int printo(u16 x)	
{
	if (x == 0)
		prints("0");
	else
	{
		putc('0');		
		rpu(x, 8);
	}
	putc(' ');
}

// print hex
int printx(u16 x)
{
	if (x == 0)
		prints("0x0");
	else
	{
		putc('0'); putc('x');	
		rpu(x, 16);
	}
	putc(' ');
}

// print format
int printf(char *fmt, ...)
{
	char *cp = fmt;
	short *ip = &fmt + 1;
	
	while (*cp != '\0')
	{
		// check for format indicator
		if (*cp == '%')		
		{
			// increment char pointer
			cp++;		

			switch (*cp)
			{
				case 'c':
					putc(*ip);
					break;
				case 's':
					prints(*ip);
					break;
				case 'u':
					printu(*ip);
					break;
				case 'd':
					printd(*ip);
					break;
				case 'o':
					printo(*ip);
					break;
				case 'x':
					printx(*ip);
					break;
				default:
					prints("error");
			}

			// go to next item on stack
			ip++;	
		}
		else
		{
			putc(*cp);
			if (*cp == '\n')	
				putc('\r');
		}		
		
		cp++; 			
	
	}	
}

char *gets(char *output)
{
	char c = 0;
	int i = 0;

	printf("enter gets\n");

	while(c != '\n')
	{
		c = getc();
		putc(c);
		output[i] = c;
		i++;
	}

	return output;	
}
