#include <stdio.h>  // for printf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// local variable declarations
	int kvd_a = 5;

	// function prototypes
	void change_count(void);

	// code
	printf("\n");
	printf("kvd_a = %d\n\n", kvd_a);

	// kvd_local_count declared in change_count() is an ordinary
	// local variable, and hence will not retain its value from
	// previous calls. This is because it is allocated new memory
	// (and in this case, as well as quite often, re-initialized)
	// each time change_count() is called.

	change_count();
	change_count();
	change_count();

	return(0);
}

void change_count(void)
{
	// local variable declarations
	int kvd_local_count = 0;

	// code
	kvd_local_count = kvd_local_count + 1;

	printf("local count = %d\n", kvd_local_count);
}
