/*******************************************************\
					Nathan VelaBorja
				    December 3, 2016
						l2u.c
\*******************************************************/

#include "ucode.c"
#include "user.h"

int main(int argc, char *argv[])
{
	int fd1, fd2, n, standardOut = FALSE;
	char c;

	if (DEBUGLVL >= 1)
		printf("Enter VBl2u.\n");

	// If no tokens, use stdin and stdout
	if (argc == 1)
	{
		fd1 = 0;
		fd2 = 1;
		standardOut = TRUE;
	}

	// If one token, use file and stdout
	else if (argc == 2)
	{
		fd1 = open(argv[1], O_RDONLY);
		fd2 = 1;
		standardOut = TRUE;
	}

	// If two fokens, use files for in and out
	else
	{
		fd1 = open(argv[1], O_RDONLY);
		fd2 = open(argv[2], O_WRONLY|O_CREAT);
	}

	// Check files
	if (fd1 < 0)
	{
		printf("Error opening file '%s'.\n", argv[1]);
		return FN_FAILURE;
	}
	if (fd2 < 0)
	{
		printf("Error opening file '%s'.\n", argv[2]);
		return FN_FAILURE;
	}

	// Start converting

	n = read(fd1, &c, 1);

	while (n)
	{
		if (c > 96 && c < 123)
			c -= 32;

		if (standardOut)
			putc(c);
		else
			write(fd2, &c, 1);

		n = read(fd1, &c, 1);
	}

	close(fd1);
	close(fd2);

	return FN_SUCCESS;
}
