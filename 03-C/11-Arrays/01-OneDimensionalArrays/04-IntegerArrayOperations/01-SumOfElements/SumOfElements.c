#include <stdio.h>

#define COUNT 10

int main(void)
{
	// variable declarations
	int kvd_iArray[COUNT];
	int kvd_i, kvd_num, kvd_sum = 0;

	// code
	printf("\n\n");

	printf("enter %d integers for kvd_iArray[]:\n", COUNT);
	for (kvd_i = 0; kvd_i < COUNT; kvd_i++)
	{
		scanf("%d", &kvd_num);
		kvd_iArray[kvd_i] = kvd_num;
	}

	for (kvd_i = 0; kvd_i < COUNT; kvd_i++)
		kvd_sum = kvd_sum + kvd_iArray[kvd_i];

	printf("\n\n");

	printf("sum of all elements in kvd_iArray[] = %d\n\n", kvd_sum);

	return(0);
}
