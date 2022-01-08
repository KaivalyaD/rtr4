#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[] = { 100, 90, 80, 70, 60, 50, 40, 30, 20, 10 };
	int *kvd_ptr = NULL, kvd_i;

	// code
	// using kvd_iArray as a pointer (the uncommon way)
	printf("\n\n");
	printf("accessing kvd_iArray[] using kvd_iArray as a pointer:\n\n");
	for (kvd_i = 0; kvd_i < sizeof(kvd_iArray) / sizeof(int); kvd_i++)
		printf("\t*(kvd_iArray + %d) = %d; at address = 0x%p\n", kvd_i, *(kvd_iArray + kvd_i), (kvd_iArray + kvd_i));

	// using kvd_ptr as an alias to kvd_iArray
	kvd_ptr = kvd_iArray;

	// and using kvd_ptr as we would otherwise use an array variable (i.e. indexing)
	printf("\n\n");
	printf("accessing kvd_iArray[] using kvd_ptr as an alias array variable:\n\n");
	for (kvd_i = 0; kvd_i < sizeof(kvd_iArray) / sizeof(int); kvd_i++)
		printf("\tkvd_ptr[%d] = %d; at address = 0x%p\n", kvd_i, kvd_ptr[kvd_i], &kvd_ptr[kvd_i]);

	printf("\n\n");
	return(0);
}
