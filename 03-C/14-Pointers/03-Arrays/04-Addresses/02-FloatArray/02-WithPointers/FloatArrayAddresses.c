#include <stdio.h>

int main(void)
{
	// variable declarations
	float kvd_fArray[10], *kvd_ptr = NULL;
	int kvd_i;

	// code
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_fArray[kvd_i] = 2.71f * (kvd_i + 1.0f);

	kvd_ptr = kvd_fArray;

	printf("\n\n");
	printf("elements in kvd_fArray[] and their addresses:\n\n");
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("\tkvd_fArray[%d] = %f; at address = 0x%p\n", kvd_i, *(kvd_ptr + kvd_i), (kvd_ptr + kvd_i));

	printf("\n\n");
	return(0);
}
