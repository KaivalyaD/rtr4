#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i;

	// code
	printf("\n\n");

	printf("printing digits from 10 to 1:\n\n");

	kvd_i = 10;
	while (kvd_i >= 1)
	{
		printf("\t%d\n", kvd_i);
		kvd_i--;
	}

	printf("\n\n");

	return(0);
}
