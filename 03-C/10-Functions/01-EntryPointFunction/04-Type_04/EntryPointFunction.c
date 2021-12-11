#include <stdio.h>  // for printf()

// Type-04 Entry Point: returns an int, takes 2 arguments -- the command line arguments themselves, and their count
int main(int kvd_argc, char* kvd_argv[])  // can be "char argv[][]" or "char** argv"
{
	// variable declarations
	int kvd_i;

	// code
	printf("\n\n");

	printf("hello, world!\n\n");

	printf("the number of command line arguments entered = %d\n\n", kvd_argc);

	printf("where the following commands were passed:\n");
	for (kvd_i = 0; kvd_i < kvd_argc; kvd_i++)
		printf("argument #%d = \"%s\"\n", kvd_i+1, kvd_argv[kvd_i]);

	printf("\n\n");
	return(0);
}
