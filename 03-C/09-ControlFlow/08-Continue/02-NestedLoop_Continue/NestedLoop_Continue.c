#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	printf("the outer loop prints odd numbers between 1 and 10, while the inner loop prints even numbers between 1 and 10\n\n");

	for (kvd_i = 1; kvd_i <= 10; kvd_i++)
	{
		if (kvd_i % 2 != 0)
		{
			printf("kvd_i = %d\n", kvd_i);
			printf("------------\n");

			for (kvd_j = 1; kvd_j <= 10; kvd_j++)
			{
				if (kvd_j % 2 != 0)
					continue;
				printf("\tkvd_j = %d\n", kvd_j);
			}

			printf("\n\n");
		}
	}

	printf("\n\n");

	return(0);
}
