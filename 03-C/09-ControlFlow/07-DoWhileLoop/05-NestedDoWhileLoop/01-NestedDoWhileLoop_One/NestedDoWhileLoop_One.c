#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	kvd_i = 1;
	do
	{
		printf("kvd_i = %d\n", kvd_i);
		printf("------------\n");

		kvd_j = 1;
		do
		{
			printf("\tkvd_j = %d\n", kvd_j);
			kvd_j++;
		} while (kvd_j <= 5);

		kvd_i++;
	} while (kvd_i <= 10);

	return(0);
}
