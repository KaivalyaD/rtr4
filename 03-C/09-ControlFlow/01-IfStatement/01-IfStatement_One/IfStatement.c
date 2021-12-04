#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_a, kvd_b, kvd_p;

	//code
	kvd_a = 9;
	kvd_b = 30;
	kvd_p = 30;

	printf("\n\n");

	if (kvd_a < kvd_b)
	{
		printf("a is less than b\n\n");
	}
	if (kvd_b != kvd_p)
	{
		printf("b is not equal to p\n\n");
	}

	printf("all comparisons have been done\n\n");

	return(0);
}
