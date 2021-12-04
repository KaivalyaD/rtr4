#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i;

	// code
	printf("\n\n");

	printf("printing digits from 1 to 10:\n\n");

	kvd_i = 1;
	do
	{
		printf("\t%d\n", kvd_i);
		kvd_i++;
	} while (kvd_i <= 10);

	printf("\n\n");
}
