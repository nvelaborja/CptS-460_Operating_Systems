// ==========================================================
//                    sh Program:
// YOUR sh must support
// (1). logout/exit :  terminate
// (2). simple command exceution, e.g. ls, cat, ....
// (3). I/O redirection:  e.g. cat < infile;  cat > outfile, etc.
// (4). (MULTIPLE) PIPEs: e.g. cat file | l2u | grep LINE 
// ===========================================================

/*******************************************************\
					Nathan VelaBorja
				    December 2, 2016
						 sh.c
\*******************************************************/

#include "ucode.c"
#include "user.h"

#define NCMD 12

char *commands[NCMD] = {"ls", "cd", "cat", "more", "grep", "cp", "l2u", "creat", "rm", "mkdir", "rmdir", "logout"};
char uInput[INPUTLEN], command[INPUTLEN][INPUTLEN], buffer[BUFLEN], rcmd[INPUTLEN];
int nTokens;

int main(int argc, char *argv[])
{
	printf("Welcome to VBSH!\n\n");

	while (1)
	{
		printf("\n");

		// First clear globals so no past command interferes with this one
		ClearGlobals();

		// Get user command
		if (!GetInput())
			return FN_FAILURE;

		if (DEBUGLVL >= 1)
			printf("Input: '%s'.\n", uInput);

		// Tokenize input
		Tokenize();

		if (DEBUGLVL >= 1)
			PrintTokenResults();

		// Execute command
		if (!Execute())
			printf("Command Execution Failed.\n");

		printf("\n");
		// Do it all over again!
	}

	return FN_SUCCESS;
}

int Execute()
{
	char *cmd, rFile[BUFLEN], cd[INPUTLEN];
	int cmdIndex, pid, i, status = 0;
	int containsPipe = 0, containsRedirection = 0;

	if (DEBUGLVL >= 2)
		printf("VBSH: Enter Execute().\n");

	strcpy(cmd, command[0]);

	if (!(cmdIndex = FindCommandIndex(cmd)))
	{
		printf("No command '%s' found.\n", cmd);
		return FN_FAILURE;
	}

	// Handle lougout manually
	if (cmdIndex == 12)
	{
		if (DEBUGLVL >= 1)
				printf("Logging out.\n");
		exit(EXIT_SUCCESS);
	}

	// Handle cd manually
	if (cmdIndex == 2)
	{
		// Check for empty cd.. go to root
		if (!command[1][0])
		{
			if (DEBUGLVL >= 1)
				printf("Changing Directory to root.\n");

			chdir("/");
		}

		// Check absolute path, just call chdir directly
		else if (command[1][0] == '/')
		{
			if (DEBUGLVL >= 1)
				printf("Changing Directory to '%s'.\n", command[1]);

			chdir(command[1]);
		}

		// Otherwise we need to append path to cwd
		else
		{
			getcwd(cd);
			strcat(cd, command[1]);

			if (DEBUGLVL >= 1)
				printf("Changing Directory to '%s'.\n", command[1]);

			chdir(cd);
		}

		return FN_SUCCESS;
	}

	// Command (leading) is valid
	pid = fork();

	// Fork failure
	if (pid < 0)
	{
		if (DEBUGLVL >= 1)
			printf("Fork failure.\n");
		return FN_FAILURE;
	}

	// Parent
	if (pid)
	{
		if (DEBUGLVL >= 1)
			printf("Forked child. Waiting for child to die.\n");

		pid = wait(&status);

		if (DEBUGLVL >= 1)
			printf("Child dead, waking up.\n");
	}
	// Child
	else
	{
		containsPipe = ContainsPipe(uInput);
		if (containsPipe)
		{
			if (DEBUGLVL >= 1)
				printf("Command '%s' contains a pipe.\n", uInput);
			Pipe(uInput);
		}
		else
		{
			if (DEBUGLVL >= 1)
				printf("Command '%s' contains no pipe.\n", uInput);

			containsRedirection = ContainsRedirection(uInput);
			if (containsRedirection)
			{
				if (DEBUGLVL >= 1)
					printf("Command '%s' contains an io redirection. r=%d.\nExecuting '%s'.\n", uInput, containsRedirection, rcmd);
				Redirect(uInput, containsRedirection);
				exec(rcmd);
			}
			else 
			{
				if (DEBUGLVL >= 1)
					printf("Command '%s' contains no io redirection.\n", uInput);

				exec(uInput);
			}
		}
	}

	return FN_SUCCESS;
}

int Pipe(char *cmd)
{
	int pd[2], pid, i = 0;
	char pHead[INPUTLEN], pTail[INPUTLEN];
	int containsPipe = 0, containsRedirection = 0;

	if (DEBUGLVL >= 1)
		printf("Piping '%s'.\n", cmd);

	// Create pipe
	pipe(pd);

	// Fork a child 
	pid = fork();

	// Parent - tail
	if (pid)
	{
		GetTail(cmd, pTail);

		if (DEBUGLVL >= 1)
			printf("Parent process handling tail '%s'.\n", pTail);

		close(pd[1]);
		dup2(pd[0], 0);
		close(pd[0]);
		
		if (ContainsPipe(pTail))
		{
			if (DEBUGLVL >= 1)
					printf("Command '%s' contains a pipe.\n", pTail);

			Pipe(pTail);
		}
		else
		{
			containsRedirection = ContainsRedirection(pTail);
			if (containsRedirection)
			{
				if (DEBUGLVL >= 1)
					printf("Command '%s' contains an io redirection. Executing '%s'.\n", cmd, pTail);
				Redirect(pTail, containsRedirection);
			}

			exec(pTail);
		}
	}

	// Child - head
	else
	{
		GetHead(cmd, pHead);

		if (DEBUGLVL >= 1)
			printf("Child process handling head '%s'.\n", pHead);

		close(pd[0]);
		dup2(pd[1],1);
		close(pd[1]);

		containsRedirection = ContainsRedirection(pHead);
		if (containsRedirection)
		{
			if (DEBUGLVL >= 1)
					printf("Command '%s' contains an io redirection. Executing '%s'.\n", cmd, pHead);
			Redirect(pHead, containsRedirection);

			exec(rcmd);
		}
		else
		{
			exec(pHead);
		}
	}

	return FN_SUCCESS;
}

int Redirect(char *cmd, int redirect)
{
	char next[INPUTLEN], cmdCopy[INPUTLEN];
	int i, j = 0, k, length = strlen(cmd);

	if (DEBUGLVL >= 1)
		printf("Command '%s'  r=%d, length=%d.\n", cmd, redirect, length);

	// Get rhs of redirect
	strcpy(cmdCopy, cmd);

	for (i = 0; i < length; i++)
	{
		if (cmdCopy[i] == '<' || cmdCopy[i] == '>')
		{
			cmd[i-1] = 0;

			// Copy lhs to rcmd
			k = i - 1;
			strncpy(rcmd, cmd, k);


			// Move to beginning of next token
			if (cmdCopy[i] == '>' && cmdCopy[i+1] == '>')
				i++;
			i += 2;

			// Copy the rest to next[]
			strcpy(next, cmdCopy + i);
		}
	}

	if (DEBUGLVL >= 1)
		printf("Redirecting command '%s' to '%s'. r=%d.\n", cmd, next, redirect);

	// Switch statement was not working here.. try ifs?

	if (redirect == 1)				// input redirect
	{
		close(0);

		if (open(next, O_RDONLY) != 1)
		{
			if (DEBUGLVL >= 2)
				printf("Failed to open file '%s'.\n", next);
		}
		else
			printf("Successfully opened file '%s'.\n", next);
	}
	else if (redirect == 2)			// output redirect
	{
		close(1);
		
		if (DEBUGLVL >= 2)
			write(2, "Opening file.\n", strlen("Opening file.\n"));		// Need to use write() since we just called close(1);

		if (open(next, O_WRONLY | O_CREAT) != 1)
		{
			if (DEBUGLVL >= 1)
				write(2, "Failed to open file.\n", strlen("Failed to open file.\n"));
		}
		else if (DEBUGLVL >= 1)
			write(2, "Successfully opened file.\n", strlen("Successfully opened file.\n"));
	}
	else if (redirect == 3)			// output redirect (append)
	{
		close(1);

		if (DEBUGLVL >= 2)
			write(2, "Opening file.\n");

		if (open(next, O_APPEND | O_WRONLY | O_CREAT) != 1)
		{
			if (DEBUGLVL >= 2)
				write(2, "Failed to open file.\n", strlen("Failed to open file.\n"));
		}
		else if (DEBUGLVL >= 2)
			write(2, "Successfully opened file.\n", strlen("Successfully opened file.\n"));
	}
	else
	{
		printf("IO Redirection Error.\n");
		return FN_FAILURE;
	}

	return FN_SUCCESS;
}

// Finds index of command, offset by +1. Index 0 is reserved for failure
int FindCommandIndex(char *cmd)
{
	int i;

	if (DEBUGLVL >= 2)
		printf("VBSH: Enter FindCommandIndex().\n");

	for (i = 0; i < NCMD; i++)
	{
		if (!strcmp(commands[i], cmd))
		{
			if (DEBUGLVL >= 1)
				printf("Command '%s' found in '%s'.\n", commands[i], uInput);
			return i+1;
		}
	}

	return FN_FAILURE;
}

int GetInput()
{
	char cwd[MAX_PATH];

	if (DEBUGLVL >= 2)
		printf("VBSH: Enter GetInput().\n");

	// Get current working directory
	getcwd(cwd);

	// Prompt for input
	printf("VBSH:%s# ", cwd);

	// Catch input
	gets(uInput);

	if (!uInput)
		return FALSE;

	return TRUE;
}

int ContainsRedirection(char *cmd)
{
	char cmdCopy[INPUTLEN];
	int i, length = strlen(cmd);

	if (DEBUGLVL >= 2)
		printf("VBSH: Enter ContainsRedirection().\n");

	for (i = 0; i < length; i++)
	{
		if (cmd[i] == '<')	// input redirect
			return 1;
		if (cmd[i] == '>')	// output redirect
		{
			if (cmd[i+1] == '>') // output redirect (append)
				return 3;

			return 2;
		}
	}

	return FALSE;
}

int ContainsPipe(char *cmd)
{
	int i, length = strlen(cmd);

	if (DEBUGLVL >= 2)
		printf("VBSH: Enter ContainsPipe().\n");


	for (i = 0; i < length; i++)
	{
		if (cmd[i] == '|')
			return TRUE;
	}

	return FALSE;
}

// Tokenizes uInput by delimiter ' ', puts contents into command[][]
int Tokenize()
{
	char copy[INPUTLEN];						// Make a copy of uInput just incase we need it later
	char *token;
	int i = 0;

	if (DEBUGLVL >= 2)
		printf("VBSH: Enter Tokenize().\n");

	strcpy(copy, uInput);

	token = strtok(copy, " ");

	if (DEBUGLVL >= 2)
		printf("Token: %s.\n", token);

	while (token)
	{
		strcpy(command[i], token);
		token = strtok(0, " ");

		if (DEBUGLVL >= 2)
			printf("Token: %s.\n", token);

		i++;
	}

	nTokens = i;

	if (DEBUGLVL >= 2)
		printf("nTokens: %d.\n", nTokens);

	return FN_SUCCESS;
}

int ClearGlobals()
{
	int i;

	if (DEBUGLVL >= 2)
		printf("VBSH: Enter ClearGlobals().\n");

	for (i = 0; i < INPUTLEN; i++)
		MemoryClear(command[i], INPUTLEN);

	MemoryClear(uInput, INPUTLEN);
	MemoryClear(buffer, INPUTLEN);

	nTokens = 0;

	return FN_SUCCESS;
}

int MemoryClear(char *mem, int size)
{
	int i;

	if (DEBUGLVL >= 2)
		printf("VBSH: Enter MemoryClear().\n");

	for (i = 0; i < size; i++)
		mem[i] = 0;

	return FN_SUCCESS;
}

int PrintTokenResults()
{
	int i;

	printf("Command Tokens:\n");

	for (i = 0; i < nTokens; i++)
	{
		printf("   (%d): '%s'.\n", i+1, command[i]);
	}

	return FN_SUCCESS;
}

int GetHead(char *cmd, char *head)
{
	int i, length = strlen(cmd);

	for (i = 0; i < length; i++)
	{
		if (cmd[i] == '|')
		{
			strcpy(head, cmd);
			head[i - 1] = 0;

			return FN_SUCCESS;
		}
	}

	return FN_FAILURE;
}

int GetTail(char *cmd, char *tail)
{
	int i, length = strlen(cmd);

	for (i = 0; i < length; i++)
	{
		if (cmd[i] == '|')
		{
			strcpy(tail, cmd + i + 2); 			// +2 to skip the pipe symbol and the space following

			return FN_SUCCESS;
		}
	}

	return FN_FAILURE;
}