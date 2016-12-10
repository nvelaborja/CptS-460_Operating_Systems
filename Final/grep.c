/*******************************************************\
					Nathan VelaBorja
				    December 3, 2016
						grep.c
\*******************************************************/

#include "ucode.c"
#include "user.h"

int main(int argc, char *argv[])
{
	int fd, lineNumber = 1, gfeof;
	char pattern[INPUTLEN], fline[BUFLEN];

	if (DEBUGLVL >= 1)
		printf("Enter VBgrep.\n");

	if (argc < 2)
	{
		printf("VBgrep: invalid input\n");
		return FN_FAILURE;
	}

	gfeof = FALSE;
	strcpy(pattern, argv[1]);

	if (DEBUGLVL >= 1)
		printf("Looking for pattern: '%s'.\n", pattern);

	if (argc == 2)
		fd = 0;
	else
	{
		if (DEBUGLVL >= 1)
			printf("Opening '%s' for reading.\n", argv[2]);

		fd = open(argv[2], O_RDONLY);

		if (!fd)
		{
			printf("VBgrep: can't find file '%s'.\n", argv[2]);
			return FN_FAILURE;
		}

		if (DEBUGLVL >= 1)
			printf("Opened '%s' successfully. fd=%d\n", argv[2], fd);
	}

	while (!gfeof)
	{
		Grep_ClearLine(fline);
		Grep_GetLine(fd, fline, &gfeof);
		if (Grep_ContainsPattern(fline, pattern))
			Grep_PrintFoundLine(lineNumber, fline);
		lineNumber++;
	}

	close(fd);

	return FN_SUCCESS;
}

int Grep_ClearLine(char *fline)
{
	int i;

	for (i = 0; i < BUFLEN; i++)
		fline[i] = 0;

	return FN_SUCCESS;
}

int Grep_GetLine(int fd, char *fline, int *gfeof)
{
	int i = 0, n;

	while (i < BUFLEN)
	{
		n = read(fd, &fline[i], 1);

		if (!n)
		{
			if (DEBUGLVL >= 2)
				printf("End of file found. n=%d\n", n);

			*gfeof = TRUE;
			return FN_SUCCESS;
		}

		if (fline[i] == '\n' || fline[i] == '\r')
		{
			if (DEBUGLVL >= 2)
				printf("Newline char found. c=%d\n", (int)fline[i]);

			return FN_SUCCESS;
		}

		i++;
	}
}

int Grep_ContainsPattern(char *fline, char *pattern)
{
	return strstr(fline, pattern);
}

int Grep_PrintFoundLine(int lineNumber, char *fline)
{
	printf("%d: %s\n", lineNumber, fline);

	return FN_SUCCESS;
}