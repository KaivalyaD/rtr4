#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	for (kvd_i = 1; kvd_i <= 10; kvd_i++)
	{
		printf("kvd_i = %d\n", kvd_i);
		printf("------------\n\n");

		for (kvd_j = 1; kvd_j <= 5; kvd_j++)
			printf("\tkvd_j = %d\n", kvd_j);

		printf("\n\n");
	}

	return(0);
}
