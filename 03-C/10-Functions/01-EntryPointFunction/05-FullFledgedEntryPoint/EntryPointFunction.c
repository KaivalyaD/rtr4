#include <stdio.h>  // for printf()

// Full Fledged Entry Point: returns an int, takes 3 arguments -- the
// command line arguments, their count, and the environment variables
// defining the environment in which this instance of current program
// was executed
int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// variable declarations
	int kvd_i;

	// code
	printf("\n\n");

	printf("hello, world!\n\n");

	printf("number of command line arguments = %d\n\n", kvd_argc);

	printf("the passed arguments are:\n");
	for (kvd_i = 0; kvd_i < kvd_argc; kvd_i++)
	{
		printf("argument #%d = \"%s\"\n", kvd_i + 1, kvd_argv[kvd_i]);
	}
	printf("\n\n");

	printf("first 20 environment variables passed to this program are:\n");
	for (kvd_i = 0; kvd_i < 20; kvd_i++)
	{
		printf("environment variable #%d = \"%s\"\n", kvd_i + 1, kvd_envp[kvd_i]);
	}
	printf("\n\n");

	return(0);
}
