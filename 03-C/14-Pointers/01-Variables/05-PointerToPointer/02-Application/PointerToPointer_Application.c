#include <stdio.h>

int main(void)
{
	// variable declarations
	
	// method 1 of declaration:
	//	**pptr represents an integer;
	//	*pptr represents an integer pointer; and
	//	pptr represents a pointer to an integer pointer
	int kvd_num, *kvd_ptr = NULL, **kvd_pptr = NULL;

	// code
	kvd_num = 10;

	printf("\n\n");

	printf("before kvd_ptr = &kvd_num:\n\n");
	printf("\tvalue of kvd_num = %d\n", kvd_num);
	printf("\taddress of kvd_num = &kvd_num = 0x%p\n", &kvd_num);
	printf("\tvalue at address of kvd_num = *(&kvd_num) = %d\n\n", *(&kvd_num));

	kvd_ptr = &kvd_num;

	printf("\n\n");

	printf("after kvd_ptr = &kvd_num and before kvd_pptr = &kvd_ptr:\n\n");
	printf("\tvalue of kvd_num = %d\n", kvd_num);
	printf("\taddress of kvd_num = kvd_ptr = 0x%p\n", kvd_ptr);
	printf("\taddress of kvd_ptr = &kvd_ptr = 0x%p\n", &kvd_ptr);
	printf("\tvalue at address of kvd_ptr = *(&kvd_ptr) = 0x%p\n", *(&kvd_ptr));
	printf("\tvalue at address of kvd_num = *kvd_ptr = *(*(&kvd_ptr)) = %d\n\n", *(*(&kvd_ptr)));

	// assigning the address of kvd_ptr to kvd_pptr
	kvd_pptr = &kvd_ptr;

	printf("\n\n");

	printf("after kvd_pptr = &kvd_ptr:\n\n");
	printf("\tvalue of kvd_num = %d\n", kvd_num);
	printf("\taddress of kvd_num = kvd_ptr = 0x%p\n", kvd_ptr);
	printf("\taddress of kvd_ptr = kvd_pptr = 0x%p\n", kvd_pptr);
	printf("\tvalue at address of kvd_ptr = *kvd_pptr = 0x%p\n", *kvd_pptr);
	printf("\tvalue at address of kvd_num = *kvd_ptr = **kvd_pptr = %d\n\n", **kvd_pptr);

	return 0;
}
