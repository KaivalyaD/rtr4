#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	// function declarations
	void MathematicalOperations(int, int, int *, int *, int *, int *, int *);

	// variable declarations
	int kvd_a, kvd_b;
	int *kvd_sum = NULL, *kvd_diff = NULL, *kvd_prod = NULL, *kvd_quot = NULL, *kvd_rem = NULL;
	
	// code
	printf("\n\n");
	printf("enter a number: ");
	scanf("%d", &kvd_a);
	printf("enter another number: ");
	scanf("%d", &kvd_b);

	printf("\n\n");
	
	kvd_sum = (int *)malloc(sizeof(int) * 1);
	if (!kvd_sum)
	{
		printf("malloc(): failed to allocate the space for the sum\n\n");
		return 1;
	}
	
	kvd_diff = (int *)malloc(sizeof(int) * 1);
	if (!kvd_diff)
	{
		printf("malloc(): failed to allocate the space for the difference\n\n");
		return 1;
	}

	kvd_prod = (int *)malloc(sizeof(int) * 1);
	if (!kvd_prod)
	{
		printf("malloc(): failed to allocate the space for the product\n\n");
		return 1;
	}

	kvd_quot = (int *)malloc(sizeof(int) * 1);
	if (!kvd_quot)
	{
		printf("malloc(): failed to allocate the space for the quotient\n\n");
		return 1;
	}

	kvd_rem = (int *)malloc(sizeof(int) * 1);
	if (!kvd_rem)
	{
		printf("malloc(): failed to allocate the space for the remainder\n\n");
		return 1;
	}

	MathematicalOperations(
		kvd_a, kvd_b,
		kvd_sum,
		kvd_diff,
		kvd_prod,
		kvd_quot, kvd_rem
	);

	printf("results:\n\n");
	printf("\tsum = %d\n", *kvd_sum);
	printf("\tdifference = %d\n", *kvd_diff);
	printf("\tproduct = %d\n", *kvd_prod);
	printf("\tquotient = %d\n", *kvd_quot);
	printf("\tremainder = %d\n\n", *kvd_rem);

	if (kvd_sum)
	{
		free(kvd_sum);
		kvd_sum = NULL;
		printf("freed and cleaned kvd_sum\n");
	}

	if (kvd_diff)
	{
		free(kvd_diff);
		kvd_diff = NULL;
		printf("freed and cleaned kvd_diff\n");
	}

	if (kvd_prod)
	{
		free(kvd_prod);
		kvd_prod = NULL;
		printf("freed and cleaned kvd_prod\n");
	}

	if (kvd_quot)
	{
		free(kvd_quot);
		kvd_quot = NULL;
		printf("freed and cleaned kvd_quot\n");
	}

	if (kvd_rem)
	{
		free(kvd_rem);
		kvd_rem = NULL;
		printf("freed and cleaned kvd_rem\n");
	}

	printf("freed and cleaned all dynamically allocated memory\n\n");
	return 0;
}

void MathematicalOperations(
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

	if (kvd_y != 0)
	{
		*kvd_quot = kvd_x / kvd_y;
		*kvd_rem = kvd_x % kvd_y;
	}
	else
	{
		*kvd_quot = 0;
		*kvd_rem = 0;
	}
}
