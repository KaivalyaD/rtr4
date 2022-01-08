#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[10], kvd_i;

	// code
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_iArray[kvd_i] = 3 + (kvd_i + 1) * 2;

	printf("\n\n");
	printf("elements in kvd_iArray:\n\n");
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("kvd_iArray[%d] = %d; at address = 0x%p\n", kvd_i, kvd_iArray[kvd_i], &kvd_iArray[kvd_i]);

	printf("\n\n");
	return(0);
}
