/*******************************************************\
					Nathan VelaBorja
				    December 3, 2016
						  cp.c
\*******************************************************/

#include "ucode.c"
#include "user.h"

int main(int argc, char *argv[])
{
	char old[BUFLEN], new[BUFLEN];
	int fd1, fd2, n, standardOut = FALSE;
	char c;

	if (DEBUGLVL >= 1)
		printf("Enter VBcp.\n");

	// Make sure files are specified
	if (argc < 3)
	{
		printf("Invalid input.\n");
	}

	fd1 = open(argv[1], O_RDONLY);
	fd2 = open(argv[2], O_WRONLY|O_CREAT);

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
		write(fd2, &c, 1);

		n = read(fd1, &c, 1);
	}

	close(fd1);
	close(fd2);

	return FN_SUCCESS;
}