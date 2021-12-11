#include <stdio.h>  // for printf()
#include <stdlib.h>  // for exit()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp)
{
	// variable declarations
	int kvd_i;

	// code
	if (kvd_argc != 4)
	{
		printf("\n\n");

		printf("invalid usage!\n");
		printf("correct usage: CommandLineArgumentsApplication[.exe] <first name> <middle name> <surname>\n\n");
		exit(1);
	}

	/*********************************************************************
	* This program accepts exactly 3 strings from the user as the user's *
	* name, and echoes it to the command line.												 *
	**********************************************************************/

	printf("\n\n");

	printf("your full name: ");
	for (kvd_i = 1; kvd_i < kvd_argc; kvd_i++)
	{
		printf("%s ", kvd_argv[kvd_i]);
	}

	printf("\n\n");
	return(0);
}
