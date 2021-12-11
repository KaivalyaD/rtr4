#include <stdio.h>  // for the declaration of printf()
#include <ctype.h>  // for the declaration of atoi()
#include <stdlib.h>  // for the declaration of exit()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// variable declarations
	int kvd_i, kvd_num, kvd_sum = 0;

	// code
	if (kvd_argc == 1)
	{
		printf("\n\n");

		printf("no numbers provided for addition!\n");
		printf("usage: CommandLineArgumentsApplication[.exe] <first number> <second number> ...\n\n");
		exit(0);
	}

	/*****************************************************************
	* This program accepts all numeric values passed as command line *
	* arguments to it, and prints their sum.						 *
	******************************************************************/
	printf("\n\n");

	printf("sum of all command line arguments is: ");
	for (kvd_i = 1; kvd_i < kvd_argc; kvd_i++)
	{
		kvd_num = atoi(kvd_argv[kvd_i]);
		kvd_sum = kvd_sum + kvd_num;
	}

	printf("%d\n\n", kvd_sum);
	return(0);
}
