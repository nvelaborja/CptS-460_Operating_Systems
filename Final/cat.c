/*******************************************************\
					Nathan VelaBorja
				    December 3, 2016
						cat.c
\*******************************************************/

#include "ucode.c"
#include "user.h"

int main(int argc, char *argv[])
{
	int fd, n, i, redirected = FALSE;
	char c, line[BUFLEN], tty[64];
	STAT st0, st1, st_tty;

	if (DEBUGLVL >= 1)
		printf("Enter VBcat.\n");

	// Check out if files are specified or not
	if (argc == 1)
		fd = 0;
	else
		fd = open(argv[1], O_RDONLY);

	// Check file
	if (fd < 0)
	{
		printf("Error opening file '%s'.\n", argv[1]);
		return FN_FAILURE;
	}

	// Start reading

	if (fd)	// File
	{
		n = read(fd, &c, 1);

		while (n)
		{
			putc(c);

			n = read(fd, &c, 1);
		}
	}
	else	// Stdin - wasn't really sure what to do here, so I copied linux functionality
	{
		// Check if IO is being redirected
		redirected = IsOutRedirected();

		if (redirected)
		{
			while (1)
			{
				n = read(fd, &c, 1);

				if (c == 4 || !n)				// Control-d or EOF
				{
					return FN_SUCCESS;
				}

				write(2, &c, 1);		// Write to console
				if (c == '\r')
				{			
					c = '\n';
					write(2, &c, 1);
				}

				putc(c);				// Write to redirected output
			}
		}
		else{
			while(1)
			{
				n = read(fd, &c, 1);

				if (c == 13)
					c = '\n';

				if (c == 4 || !n)		// Control-d or EOF
				{
					return FN_SUCCESS;
				}

				putc(c);
			}
		}
	}

	close(fd);

	return FN_SUCCESS;
}

int IsOutRedirected()
{
	STAT stat;
	char buffer[BUFLEN];
	int n = 0;

	// Stat stdout
	fstat(1, &stat);

	if (((stat.st_mode & 0xF000) == 0x8000) ||
		((stat.st_mode & 0xF000) == 0x4000) ||
		((stat.st_mode & 0xF000) == 0xA000))
		return TRUE;

	return FALSE;
}