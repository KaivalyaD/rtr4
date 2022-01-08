#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[10], *kvd_ptr = NULL, kvd_i;

	// code
	printf("\n\n");

	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		kvd_iArray[kvd_i] = 3 + (kvd_i - 3) * 3;

	kvd_ptr = kvd_iArray;

	printf("elements and addresses in kvd_iArray[]:\n\n");
	for (kvd_i = 0; kvd_i < 10; kvd_i++)
		printf("\tkvd_iArray[%d] = %d; at addresses = 0x%p\n", kvd_i, *(kvd_ptr + kvd_i), (kvd_ptr + kvd_i));

	printf("\n\n");
	return(0);
}
