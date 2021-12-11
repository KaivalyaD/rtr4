#include <stdio.h>  // for printf() and scanf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// function prototype declarations
	int MyAddition(int, int);

	// variable declarations
	int kvd_a, kvd_b, kvd_result;

	// code
	printf("\n\n");

	printf("enter a value for kvd_a: ");
	scanf("%d", &kvd_a);
	printf("enter a value for kvd_b: ");
	scanf("%d", &kvd_b);

	kvd_result = MyAddition(kvd_a, kvd_b);

	printf("\n\n");

	printf("sum of kvd_a (= %d) and kvd_b (= %d) is %d\n\n", kvd_a, kvd_b, kvd_result);

	return(0);
}

// this time, the function computes the sum of the 2 arguments passed to it,
// and returns that sum to the caller
int MyAddition(int kvd_x, int kvd_y)
{
	return(kvd_x + kvd_y);
}
