#include <stdio.h>  // for printf() and scanf()

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// variable declarations
	unsigned int kvd_num;

	// function prototypes
	void recur(unsigned int);

	// code
	printf("\n\n");

	printf("enter a number: ");
	scanf("%d", &kvd_num);

	printf("\n\n");

	printf("output of the recursive function:\n\ncalling recur(%d)\n", kvd_num);
	recur(kvd_num);

	printf("\n\n");
	return(0);
}

void recur(unsigned int kvd_x)
{
	// code
	if (kvd_x > 0)
	{
		printf("calling recur(%d)\n", kvd_x - 1);
		recur(kvd_x - 1);
	}
	else
	{
		printf("\nbase condition became false at kvd_x = %d\n\n", kvd_x);
	}

	printf("kvd_x = %d\n", kvd_x);
}
