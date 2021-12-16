#include <stdio.h>

#define COUNT 10

int main(void)
{
	// variable declarations
	int kvd_iArray[COUNT];
	int kvd_i, kvd_num, kvd_sum = 0;

	// code
	printf("\n\n");

	/* array initialization by user input */
	printf("enter %d integers into kvd_iArray[]:\n", COUNT);
	for (kvd_i = 0; kvd_i < COUNT; kvd_i++)
	{
		scanf("%d", &kvd_num);
		kvd_iArray[kvd_i] = kvd_num;
	}

	printf("\n\n");

	/* picking out even numbers from kvd_iArray[] */
	printf("even numbers in kvd_iArray[]:\n");
	for (kvd_i = 0; kvd_i < COUNT; kvd_i++)
	{
		if ((kvd_iArray[kvd_i] % 2) == 0)
			printf("\t%d\n", kvd_iArray[kvd_i]);
	}

	printf("\n\n");

	/* picking out odd numbers from kvd_iArray[] */
	printf("odd numbers in kvd_iArray[]:\n");
	for (kvd_i = 0; kvd_i < COUNT; kvd_i++)
	{
		if ((kvd_iArray[kvd_i] % 2) != 0)
			printf("\t%d\n", kvd_iArray[kvd_i]);
	}

	return(0);
}
