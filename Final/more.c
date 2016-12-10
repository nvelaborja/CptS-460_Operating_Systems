/*******************************************************\
					Nathan VelaBorja
				    December 3, 2016
						more.c
\*******************************************************/

#include "ucode.c"
#include "user.h"

int main(int argc, char *argv[])
{
	char tty[64], c;
	int fd, status;

	if (DEBUGLVL >= 1)
		printf("Enter VBmore.\n");

	// If no file was given, we use stdin
	if (argc == 1)
	{
		if (DEBUGLVL >= 1)
		printf("Using stdin.\n");

		fd = dup(0);
		close(0);
		gettty(tty);
		open(tty, O_RDONLY);
	}
	else
		fd = open(argv[1], O_RDONLY);

	// Check if open failed
	if (fd < 0)
	{
		printf("File '%s' not found.\n", argv[1]);

		return FN_FAILURE;
	}

	// Start reading / printing
	// Always print one page first
	status = PrintPage(fd);

	if (status == ENDFILE)		// That first print page might have been it.
		return FN_SUCCESS;

	while (1)
	{
		c = getc();				// Get user input

		switch (c)
		{
			case '\r':
				status = PrintLine(fd);
				break;
			case ' ':
				status = PrintPage(fd);
				break;
			case 'q':
				putc('\n');
				return FN_SUCCESS;
				break;
		}

		if (status == ENDFILE)
			return FN_SUCCESS;
	}

	return FN_SUCCESS;
}


int PrintLine(int fd)
{
	int i, n;
	char c;

	for (i = 0; i < SCREEN_W; i++)
	{
		n = read(fd, &c, 1);		// Read 1 char

		if (!n)
			return ENDFILE;

		putc(c);

		if (c == '\n' || c == '\r')		// break on newline char - Can't tell whether \r or \n is right, so get both!
			break;

	}

	putc('\n');

	return FN_SUCCESS;
}

int PrintPage(int fd)
{
	int i, j, n;
	char c;

	for (i = 0; i < SCREEN_H; i++)
	{
		for (j = 0; j < SCREEN_W; j++)
		{
			n = read(fd, &c, 1);		// Read 1 char

			if (!n)
				return ENDFILE;

			putc(c);

			if (c == '\n' || c == '\r')		// break on newline char - Can't tell whether \r or \n is right, so get both!
				break;

		}
	}

	putc('\n');

	return FN_SUCCESS;
}