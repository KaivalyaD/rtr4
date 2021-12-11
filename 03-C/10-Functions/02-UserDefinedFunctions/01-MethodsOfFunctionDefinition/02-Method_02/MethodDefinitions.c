#include <stdio.h>  // for printf() and scanf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp)
{
	// function declarations
	int MyAddition(void);

	// variable declarations
	int kvd_result;

	// code
	kvd_result = MyAddition();  // call

	printf("\n\n");

	printf("result = %d\n\n", kvd_result);
	return(0);
}

// this time, the user-defined function returns an int but still takes
// no argument
int MyAddition(void)
{
	// variable declarations
	int kvd_a, kvd_b;

	// code
	printf("\n\n");

	printf("enter a value for kvd_a: ");
	scanf("%d", &kvd_a);
	printf("enter a value for kvd_b: ");
	scanf("%d", &kvd_b);

	return(kvd_a + kvd_b);
}
