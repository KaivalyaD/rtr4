#include <stdio.h>

int main(void)
{
	// variable declarations
	double kvd_dArray[10];
	int kvd_i;

	// code
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_dArray[kvd_i] = 7.17 - (kvd_i * 2.9091);

	printf("\n\n");
	printf("elements in kvd_dArray[] and their addresses:\n\n");
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("\tkvd_dArray[%d] = %lf; at address = 0x%p\n", kvd_i, kvd_dArray[kvd_i], &kvd_dArray[kvd_i]);
	
	printf("\n\n");
	return(0);
}
