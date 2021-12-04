#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i;

	// code
	printf("\n\n");

	printf("printing even numbers from 0 to 100:\n\n");

	for (kvd_i = 0; kvd_i <= 100; kvd_i++)
	{
		if (kvd_i % 2 != 0)
			continue;
		printf("\t%d\n", kvd_i);
	}

	printf("\n\n");

	return(0);
}
