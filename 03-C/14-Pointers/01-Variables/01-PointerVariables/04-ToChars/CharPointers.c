#include <stdio.h>

int main(void)
{
	// variable declarations
	char kvd_c, *kvd_ptr = NULL;

	// code
	kvd_c = 'G';

	printf("\n\n");

	printf("before kvd_ptr = &kvd_c:\n\n");
	printf("\tvalue of kvd_c = '%c'\n", kvd_c);
	printf("\taddress of kvd_c = &kvd_c = 0x%p\n", &kvd_c);
	printf("\tvalue at address of kvd_c = *(&kvd_c) = '%c'\n", *(&kvd_c));

	kvd_ptr = &kvd_c;

	printf("\n\n");
	printf("after kvd_ptr = &kvd_c:\n\n");
	printf("\tvalue of kvd_c = '%c'\n", kvd_c);
	printf("\taddress of kvd_c = value of kvd_ptr = 0x%p\n", kvd_ptr);
	printf("\tvalue at address of kvd_c = value at kvd_ptr = *kvd_ptr = '%c'\n", *kvd_ptr);

	printf("\n\n");
	return(0);
}
