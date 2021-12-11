#include <stdio.h>  // for printf()

int main(int kvd_argc, char** kvd_argv, char** kvd_envp)
{
	// local variable declarations
	
	// function prototypes
	void change_count(void);

	// code
	printf("\n");

	/* This time, kvd_local_count is static, meaning it will
	   retain its value across function calls. Thus, calling
	   change_count() several times will affect kvd_local_count
	   cumulatively rather than in an isolated manner like before.
	   
	   This behaivour is deceptively close to a global variable.
	   BUT, they are not given different name without a reason!
	   It is rather subtle:
	   Global variables are allocated memory at the start of a
	   program, while local static variables are allocated memory
	   the first time the function in which they are declared is
	   called. Both are however deallocated once the entire program
	   exits. */

	change_count();
	change_count();
	change_count();

	return(0);
}

void change_count(void)
{
	// local variable declarations
	static int kvd_local_count = 0;

	// code
	kvd_local_count = kvd_local_count + 1;
	printf("kvd_local_count = %d\n", kvd_local_count);
}
