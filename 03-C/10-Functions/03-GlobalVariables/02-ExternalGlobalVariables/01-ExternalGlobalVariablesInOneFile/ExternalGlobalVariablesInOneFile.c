#include <stdio.h>  // for printf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function prototypes
	void change_count(void);

	// this program about how to MISUSE the extern keyword

	// variable declarations
	extern int kvd_global_count;

	// code
	printf("\n");
	printf("value of kvd_global_count before change_count() = %d\n", kvd_global_count);
	change_count();
	printf("value of kvd_global_count after change_count() = %d\n", kvd_global_count);
	printf("\n");

	return(0);
}

/* Although kvd_global_count is declared in the global scope, it is declared
   below main(). For this reason, we need to tell main() that it exists
   somewhere in the program, for the compiler to not produce an error.
   
   Plus, the linker now has to work to find kvd_global_count and place its
   address in the blank created within main() due to the extern keyword.
   
   This method is a blessing when there are more than 1 source files in
   a program, but when there is only 1, quite often a pretty huge one,
   doing this to the reader will be putting him/her, not innocently
   but deliberately through hell.
   
   The best practice is to declare all global variables before defining
   any of the functions, and limiting the use of extern only for globals
   in other files. */

int kvd_global_count = 0;

void change_count(void)
{
	// code
	kvd_global_count = 5;
	printf("change_count(): value of global_count = %d\n", kvd_global_count);
}
