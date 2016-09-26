char *table = "0123456789ABCDEF";

void prints(char *s)	// Prints a string using putchar()
{
	// Loop through the string till we find the null character to terminate the string
	while (*s != '\0')
	{
		putc(*s);
		s++;
	}
}

int rpu(u16 x, int BASE) // Given function, returns formated number based on given base
{
	char c;
	
	if (x)
	{
		c = table[x % BASE];
		rpu(x / BASE, BASE);
		putc(c);
	}
}

int rpu32(u32 x, int BASE) // 32 bit version of rpu
{
	char c;

	if (x)
	{
		c = table[x % BASE];
		rpu32(x / BASE, BASE);
		putc(c);
	}
}

int printu(u16 x) 	// prints an unsigned integer
{
	if (x==0)
		putc('0');
	else
		rpu(x, 10);
}

int printd(int x)	// prints a signed integer
{
	// If the value is negative, print a negative sign then flip the number to be positive so rpu will work
	if (x == 0)
	{
		putc('0');
		return 1;
	}
	if (x < 0)
	{
		putc('-');
		x = -x;
	}
	rpu(x, 10);
}

int printo(u16 x)	// prints an unsigned integer in octal form
{
	if (x == 0)
		prints("0");
	else
	{
		putc('0');		// Since we're only dealing with unsigned ints, sign bit will always be 0
		rpu(x, 8);
	}
}

int printx(u16 x)
{
	if (x == 0)
		prints("0x0");
	else
	{
		putc('0'); putc('x');	// Hex format. Again, we will only be dealing with unsigned ints so sign bit will always be 0
		rpu(x, 16);
	}
}

int printl(long x)
{
	if (x == 0)
	{
		putc('0');
		return 1;
	}
	if (x < 0)
	{
		putc('-');
		x = -x;
	}
	rpu32(x, 10);
}

int printf(char *fmt, ...)
{
	char *cp = fmt;
	short *ip = &fmt + 1;
	
	while (*cp != '\0')
	{
		if (*cp == '%')		// If we find a percent, grab next char which must be c, s, u, d, o, or x
		{
			cp++;		// Move to next character
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
				case 'l':
					printl(*(u32 *)ip);
					ip++;		// ip++ will only move pointer 2 bytes, we need it to move 4 bytes if a u32 was on the stack
					break;
				default:
					prints("printf(): error with arguments");
					ip++;
			}
			ip++;			// Move to the next argument on stack
		}
		else
		{
			putc(*cp);
			if (*cp == '\n')	// If we find a new line, spit out an extra '\r'
				putc('\r');
		}		
		
		cp++; 			// Move to the next item in the input string
	
	}	
}

char *gets(char *output)
{
	char c = 0;
	int i = 0;

	while(c != 13)
	{
		c = getc();
		output[i] = c;
		i++;
	}
	output[i-1] = 0;		// Replace the terminating character with a null character

	return output;	
}
