#include <stdio.h>

int main(void)
{
	// variable declarations
	float kvd_fArray[10];
	int kvd_i;

	// code
	printf("\n\n");

	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_fArray[kvd_i] = 3.14f * (kvd_i + 1.0f);

	printf("elements and their addresses in kvd_fArray[]:\n\n");
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("\tkvd_fArray[%d] = %f; at address = 0x%p\n", kvd_i, kvd_fArray[kvd_i], &kvd_fArray[kvd_i]);

	printf("\n\n");
	return(0);
}
