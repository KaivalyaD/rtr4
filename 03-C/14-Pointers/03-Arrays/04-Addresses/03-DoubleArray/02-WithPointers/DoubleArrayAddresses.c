#include <stdio.h>

int main(void)
{
	// variable declarations
	double kvd_dArray[10], *kvd_ptr = NULL;
	int kvd_i;

	// code
	printf("\n\n");

	kvd_dArray[0] = 0.0;
	for (kvd_i = 1; kvd_i < 10; kvd_i++)
		kvd_dArray[kvd_i] = 90.18983 - (kvd_dArray[kvd_i - 1] * 0.14);

	kvd_ptr = kvd_dArray;

	printf("elements of kvd_dArray[] and their addresses:\n\n");
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("\tkvd_dArray[%d] = %lf; at address = 0x%p\n", kvd_i, *(kvd_ptr + kvd_i), (kvd_ptr + kvd_i));

	printf("\n\n");
	return(0);
}
