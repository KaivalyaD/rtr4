#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num, *kvd_ptr = NULL, *kvd_ptr_copy = NULL;

	// code
	kvd_num = 49;
	kvd_ptr = &kvd_num;

	printf("\n\n");

	printf("before kvd_ptr_copy = kvd_ptr:\n\n");
	printf("\tkvd_num = %d\n", kvd_num);
	printf("\t&kvd_num = 0x%p\n", &kvd_num);
	printf("\t*(&kvd_num) = %d\n", *(&kvd_num));
	printf("\tkvd_ptr = 0x%p\n", kvd_ptr);
	printf("\t*kvd_ptr = %d\n", *kvd_ptr);

	/*
	 * kvd_ptr is an integer pointer => it can legally point only to an integer.
	 * 
	 * kvd_ptr_copy is also an integer pointer.
	 * 
	 * right now, kvd_ptr points to kvd_num and kvd_ptr_copy points to nowhere.
	 * 
	 * if we were to copy the value of kvd_ptr into kvd_ptr_copy, kvd_ptr_copy will
	 * also start to point to kvd_num because virtual addresses of declared variables
	 * cannot change at runtime.
	 * 
	 */

	kvd_ptr_copy = kvd_ptr;

	printf("\n\n");
	printf("after kvd_ptr_copy = kvd_ptr:\n\n");
	printf("\tkvd_num = %d\n", kvd_num);
	printf("\t&kvd_num = 0x%p\n", &kvd_num);
	printf("\t*(&kvd_num) = %d\n", *(&kvd_num));
	printf("\tkvd_ptr = 0x%p\n", kvd_ptr);
	printf("\t*kvd_ptr = %d\n", *kvd_ptr);
	printf("\tkvd_ptr_copy = 0x%p\n", kvd_ptr_copy);
	printf("\t*kvd_ptr_copy = %d\n\n", *kvd_ptr_copy);

	return(0);
}
