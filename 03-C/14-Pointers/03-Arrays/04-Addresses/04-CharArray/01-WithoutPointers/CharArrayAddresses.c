#include <stdio.h>

int main(void)
{
	// variable declarations
	char kvd_cArray[11] = "HelloWorld";
	int kvd_i;

	// code
	printf("\n\n");

	printf("elements in kvd_cArray[] and their addresses:\n\n");
	for (kvd_i = 0; kvd_i < 11; kvd_i++)
		printf("\tkvd_cArray[%d] = '%c'; at address = 0x%p\n", kvd_i, kvd_cArray[kvd_i], &kvd_cArray[kvd_i]);

	printf("\n\n");
	return(0);
}
