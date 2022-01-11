#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	// variable declarations
	// it is a good programming habit to intialize any pointer
	// to NULL. This way, you can check later if it does point
	// to anything or not
	int *kvd_ptr = NULL, kvd_i;
	unsigned int kvd_iArray_length = 0;

	// code
	printf("\n\n");

	printf("enter the number of elements in your array: ");
	scanf("%d", &kvd_iArray_length);

	// allocating sizeof(int) * kvd_iArray_length bytes of memory
	// for the array, to fit kvd_iArray_length integers at runtime
	kvd_ptr = (int *)malloc(sizeof(int) * kvd_iArray_length);

	// always check if the memory has been allocated or not. If not,
	// it is generally better to exit. Here's where the NULL initialization
	// of kvd_ptr comes handy
	if (!kvd_ptr)
	{
		printf("\n\n");
		printf("malloc(): failed to allocate required memory\n\n");
		return 1;  // abortive return
	}

	printf("\n");
	printf("successfully allocated memory for %d integers\n\n", kvd_iArray_length);
	printf("\tfirst element begins at 0x%p; last element begins at 0x%p\n\n", kvd_ptr, kvd_ptr + kvd_iArray_length - 1);
	
	printf("enter %d elements in the array:\n\n", kvd_iArray_length);
	for (kvd_i = 0; kvd_i < kvd_iArray_length; kvd_i++)
	{
		printf("\t");
		scanf("%d", kvd_ptr + kvd_i);
	}

	printf("\n\n");
	printf("your array contains:\n\n");
	for (kvd_i = 0; kvd_i < kvd_iArray_length; kvd_i++)
		printf("\tkvd_ptr[%d] = %d; at address = 0x%p\n", kvd_i, *(kvd_ptr + kvd_i), kvd_ptr + kvd_i);

	// if you have taken anything from the system, you MUST return it back
	// to the system.
	if (kvd_ptr)
	{
		free(kvd_ptr);
		kvd_ptr = NULL;

		printf("\n\n");
		printf("memory pointed to by kvd_ptr freed and cleaned\n\n");
	}

	return 0;
}
