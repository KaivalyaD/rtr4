#include <stdio.h>

int main(void)
{
	// local variable declarations
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	printf("printing digits from 1 to 10, and 10 to 100\n\n");

	kvd_i = 1;
	kvd_j = 10;
	while (kvd_i <= 10, kvd_j <= 100)
	{
		printf("\t%d\t%d\n", kvd_i, kvd_j);
		kvd_i++;
		kvd_j = kvd_j + 10;
	}

	printf("\n\n");

	return(0);
}
