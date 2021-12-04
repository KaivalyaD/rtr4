#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_a, kvd_b, kvd_p;

	// code
	kvd_a = 9;
	kvd_b = 30;
	kvd_p = 30;

	// first if-else pair
	printf("\n\n");
	if (kvd_a < kvd_b)
	{
		printf("entering the first if-block\n");
		printf("kvd_a is less than kvd_b\n\n");
	}
	else
	{
		printf("entering the first else-block\n");
		printf("kvd_a is not less than kvd_b\n\n");
	}
	printf("first if-else pair completed\n\n");

	// second if-else pair
	if (kvd_b != kvd_p)
	{
		printf("entering the second if-block\n");
		printf("kvd_b is not equal to kvd_p\n\n");
	}
	else
	{
		printf("entering the second else-block\n");
		printf("kvd_b is equal to kvd_p\n\n");
	}
	printf("second if-else pair completed\n\n");

	return(0);
}
