#include <stdio.h>

#define COUNT 10

int main(void)
{
	// variable declarations
	int kvd_iArray[COUNT];
	int kvd_i, kvd_j, kvd_num, kvd_count = 0;

	// code
	printf("\n\n");

	/* array initialization through user input */
	printf("enter %d integers into kvd_iArray[]:\n", COUNT);
	for (kvd_i = 0; kvd_i < COUNT; kvd_i++)
	{
		scanf("%d", &kvd_num);

		if (kvd_num < 0)
			kvd_num = kvd_num * -1;  // there are no negative prime numbers

		kvd_iArray[kvd_i] = kvd_num;
	}

	printf("\n\n");

	/* display kvd_iArray[] */
	printf("all elements in kvd_iArray[]:\n");
	for (kvd_i = 0; kvd_i < COUNT; kvd_i++)
		printf("\t%d\n", kvd_iArray[kvd_i]);

	printf("\n\n");

	/* separating primes from composites */
	printf("all prime numbers in kvd_iArray[]:\n");
	for (kvd_i = 0; kvd_i < COUNT; kvd_i++)
	{
		for (kvd_j = 1; kvd_j <= kvd_iArray[kvd_i]; kvd_j++)
		{
			if ((kvd_iArray[kvd_i] % kvd_j) == 0)
				kvd_count++;
		}

		// A prime number has only 2 factors: 1 and the number itself.
		// An anomaly occurs when the number is 1, as it is infact
		// divisible by 1 and itself (i.e. 1 again). For this reason,
		// 1 is not considered to be prime, and this program will produce
		// a correct output for all cases.
		if (kvd_count == 2)
			printf("\t%d\n", kvd_iArray[kvd_i]);

		kvd_count = 0;  // reset the counter
	}

	printf("\n\n");

	return(0);
}
