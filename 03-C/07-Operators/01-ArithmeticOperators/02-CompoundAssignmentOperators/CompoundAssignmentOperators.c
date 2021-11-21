#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_a, kvd_b, kvd_x;

	// code
	printf("\n\nenter kvd_a (int): ");
	scanf("%d", &kvd_a);

	printf("\n\nenter kvd_b (int): ");
	scanf("%d", &kvd_b);

	printf("\n\n");

	/* the following operations follow one common set of rules:
		1. compute RHS with the current value of the variable on the LHS, and
		2. assign to the variable on LHS with the result of computation just performed */

	// to retain the value of a variable for later use, store it into another temporary variable
	kvd_x = kvd_a;
	kvd_a += kvd_b;
	printf("sum of kvd_a=%d and kvd_b=%d is %d\n", kvd_x, kvd_b, kvd_a);

	kvd_x = kvd_a;	// now, kvd_x will also contain the sum of kvd_a and kvd_b as a result of the recent change in kvd_a
	kvd_a -= kvd_b;
	printf("difference between kvd_a=%d and kvd_b=%d is %d\n", kvd_x, kvd_b, kvd_a);

	kvd_x = kvd_a;
	kvd_a *= kvd_b;
	printf("product of kvd_a=%d and kvd_b=%d is %d\n", kvd_x, kvd_b, kvd_a);

	kvd_x = kvd_a;
	kvd_a /= kvd_b;
	printf("when kvd_a=%d is divided into kvd_b=%d parts, each part gets a maximum of %d units of kvd_a\n", kvd_x, kvd_b, kvd_a);

	kvd_x = kvd_a;
	kvd_a %= kvd_b;
	printf("when kvd_a=%d is divided into kvd_b=%d parts, %d units of kvd_a remain undistributed\n\n", kvd_x, kvd_b, kvd_a);

	return(0);
}
