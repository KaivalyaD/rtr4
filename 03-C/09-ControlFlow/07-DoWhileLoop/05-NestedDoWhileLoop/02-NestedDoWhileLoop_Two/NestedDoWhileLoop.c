#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j, kvd_k;

	// code
	printf("\n\n");

	kvd_i = 1;
	do
	{
		printf("kvd_i = %d\n", kvd_i);
		printf("-----------\n");

		kvd_j = 1;
		do
		{
			printf("\tkvd_j = %d\n", kvd_j);
			printf("\t------------\n");

			kvd_k = 1;
			do
			{
				printf("\t\tkvd_k = %d\n", kvd_k);
				kvd_k++;
			} while (kvd_k <= 3);

			kvd_j++;
		} while (kvd_j <= 5);

		kvd_i++;
	} while (kvd_i <= 10);

	return(0);
}
