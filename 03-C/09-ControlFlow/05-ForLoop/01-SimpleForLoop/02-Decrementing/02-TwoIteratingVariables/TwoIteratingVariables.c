#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	printf("printing numbers from 10 to 1 and 100 to 10:\n\n");
	for (kvd_i = 10, kvd_j = 100; kvd_i >= 1, kvd_j >= 10; kvd_i--, kvd_j = kvd_j - 10)
	{
		printf("\t%d\t%d\n", kvd_i, kvd_j);
	}

	printf("\n\n");

	return(0);
}
