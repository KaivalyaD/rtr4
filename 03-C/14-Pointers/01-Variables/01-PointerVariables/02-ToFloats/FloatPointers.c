#include <stdio.h>

int main(void)
{
	// variable declarations
	float kvd_num, *kvd_ptr = NULL;

	// code
	kvd_num = 92.34f;

	printf("\n\n");

	printf("before kvd_ptr = &kvd_num:\n\n");
	printf("\tvalue of kvd_num = %f\n", kvd_num);
	printf("\taddress of kvd_num = &kvd_num = 0x%p\n", &kvd_num);
	printf("\tvalue at address of kvd_num = *(&kvd_num) = %f\n", *(&kvd_num));
	
	kvd_ptr = &kvd_num;

	printf("\n\n");
	printf("after kvd_ptr = &kvd_num:\n\n");
	printf("\tvalue of kvd_num = %f\n", kvd_num);
	printf("\taddress of kvd_num = value of kvd_ptr = 0x%p\n", kvd_ptr);
	printf("\tvalue at address of kvd_num = value at kvd_ptr = *kvd_ptr = %f\n", *kvd_ptr);

	printf("\n\n");
	return(0);
}
