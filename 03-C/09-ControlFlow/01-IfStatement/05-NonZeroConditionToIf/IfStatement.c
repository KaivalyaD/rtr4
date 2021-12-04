#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_a;

	// code
	printf("\n\n");

	kvd_a = 5;
	if (kvd_a)	// non-zero positive value
	{
		printf("if-block 1: kvd_a exists and stores %d\n\n", kvd_a);
	}

	kvd_a = -5;
	if (kvd_a)	// non-zero negative value
	{
		printf("if-block 2: kvd_a exists and stores %d\n\n", kvd_a);
	}

	kvd_a = 0;
	if (kvd_a)
	{
		printf("if-block 3: kvd_a exists and stores %d\n\n", kvd_a);
	}

	printf("all if are statements covered\n\n");

	return(0);
}
