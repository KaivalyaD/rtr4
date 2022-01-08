#include <stdio.h>

int main(void)
{
	// function prototypes
	int MathematicalOperations(int, int, int *, int *, int *, int *, int *);

	// variable declarations
	int kvd_a, kvd_b;
	int kvd_sum, kvd_diff, kvd_prod, kvd_quot, kvd_rem, kvd_ret;

	// code
	printf("\n\n");
	printf("enter an integer: ");
	scanf("%d", &kvd_a);
	printf("enter another integer: ");
	scanf("%d", &kvd_b);

	printf("\n\n");

	// MathematicalOperation() is a function that, in effect, returns 6 values
	// in total! This is one of the greatest powers of pointers!
	kvd_ret = MathematicalOperations(kvd_a, kvd_b, &kvd_sum, &kvd_diff, &kvd_prod, &kvd_quot, &kvd_rem);

	if (kvd_ret == 0)
	{
		printf("results:\n\n");
		printf("\tsum = %d\n", kvd_sum);
		printf("\tdifference = %d\n", kvd_diff);
		printf("\tproduct = %d\n", kvd_prod);
		printf("\tquotient = %d\n", kvd_quot);
		printf("\tremainder = %d\n\n", kvd_rem);
	}
	else
	{
		printf("results:\n\n");
		printf("\tsum = %d\n", kvd_sum);
		printf("\tdifference = %d\n", kvd_diff);
		printf("\tproduct = %d\n", kvd_prod);
		printf("\tcannot divide by 0\n\n");
	}

	return(0);
}

int MathematicalOperations(
		int kvd_x, int kvd_y,
		int *kvd_sum,
		int *kvd_diff,
		int *kvd_prod,
		int *kvd_quot, int *kvd_rem
	)
{
	// code
	*kvd_sum = kvd_x + kvd_y;
	*kvd_diff = kvd_x - kvd_y;
	*kvd_prod = kvd_x * kvd_y;
	
	if (kvd_y == 0)	// cannot divide by zero
	{
		// it is in general a good practice to zero-out the values
		// that were to contain the result of operations
		// that couldn't be performed
		
		*kvd_quot = 0;
		*kvd_rem = 0;
		return 1;
	}

	*kvd_quot = kvd_x / kvd_y;
	*kvd_rem = kvd_x % kvd_y;

	return 0;
}
