#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	printf("printing digits from 10 to 1, and 100 to 10:\n\n");

	kvd_i = 10;
	kvd_j = 100;
	while (kvd_i >= 1, kvd_j >= 10)
	{
		printf("\t%d\t%d\n", kvd_i, kvd_j);
		kvd_i--;
		kvd_j = kvd_j - 10;
	}

	printf("\n\n");

	return(0);
}
