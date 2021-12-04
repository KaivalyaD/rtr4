#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j, kvd_k;

	// code
	printf("\n\n");
	for (kvd_i = 1; kvd_i <= 10; kvd_i++)
	{
		printf("kvd_i = %d\n", kvd_i);
		printf("-----------\n\n");

		for (kvd_j = 1; kvd_j <= 5; kvd_j++)
		{
			printf("\tkvd_j = %d\n", kvd_j);
			printf("\t-----------\n\n");

			for (kvd_k = 1; kvd_k <= 3; kvd_k++)
				printf("\t\tkvd_k = %d\n", kvd_k);

			printf("\n\n");
		}

		printf("\n\n");
	}

	return(0);
}
