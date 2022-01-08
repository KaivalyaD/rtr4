#include <stdio.h>

enum
{
	NEGATIVE = -1,
	ZERO,
	POSITIVE
};

int main(void)
{
	// function prototypes
	int Difference(int, int, int *);

	// variable declarations
	int kvd_a, kvd_b, kvd_answer, kvd_ret;

	// code
	printf("\n\n");
	printf("enter an integer: ");
	scanf("%d", &kvd_a);
	printf("enter another integer: ");
	scanf("%d", &kvd_b);

	kvd_ret = Difference(kvd_a, kvd_b, &kvd_answer);

	printf("\n\n");
	printf("results:\n\n");
	if (kvd_ret == POSITIVE)
		printf("\tthe difference between %d and %d is positive\n\n", kvd_a, kvd_b);
	else if (kvd_ret == ZERO)
		printf("\t%d = %d\n\n", kvd_a, kvd_b);
	else
		printf("\tthe difference between %d and %d is negative\n\n", kvd_a, kvd_b);

	return(0);
}

int Difference(int kvd_x, int kvd_y, int *kvd_res)
{
	// code
	*kvd_res = kvd_x - kvd_y;
	
	if (*kvd_res > 0)
		return POSITIVE;
	else if (*kvd_res == 0)
		return ZERO;
	else
		return NEGATIVE;
}
