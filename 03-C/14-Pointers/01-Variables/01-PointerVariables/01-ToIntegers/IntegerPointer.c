#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num;
	int *kvd_ptr = NULL;  // method 1: interpretation: '*ptr' is a variable of type int, or, 'ptr' is a variable of type int*

	// code
	kvd_num = 10;

	printf("\n\n");
	
	printf("before kvd_ptr = &kvd_num:\n\n");
	printf("\tvalue of kvd_num = %d\n", kvd_num);
	printf("\taddress of kvd_num = &kvd_num = 0x%p\n", &kvd_num);
	printf("\tvalue at address of kvd_num = *(&kvd_num) = %d\n", *(&kvd_num));

	// assigning the address of kvd_num to kvd_ptr
	kvd_ptr = &kvd_num;

	printf("\n\n");
	printf("after kvd_ptr = &kvd_num:\n\n");
	printf("\tvalue of kvd_num = %d\n", kvd_num);
	printf("\taddress of kvd_num = value of kvd_ptr = 0x%p\n", kvd_ptr);
	printf("\tvalue at address of kvd_num = value at kvd_ptr = *kvd_ptr = %d\n", *kvd_ptr);

	printf("\n\n");
	return 0;
}
