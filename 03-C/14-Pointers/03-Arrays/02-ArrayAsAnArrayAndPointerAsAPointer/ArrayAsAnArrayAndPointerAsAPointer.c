#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 };
	int *kvd_ptr = NULL, kvd_i;

	// code
	// using the name of the array to access it (common approach)
	printf("\n\n");
	printf("accessing kvd_iArray[] using its name:\n\n");
	for (kvd_i = 0; kvd_i < sizeof(kvd_iArray) / sizeof(int); kvd_i++)
		printf("\tkvd_iArray[%d] = %d; at address = 0x%p\n", kvd_i, kvd_iArray[kvd_i], &kvd_iArray[kvd_i]);

	// using kvd_ptr as an alias to kvd_iArray to access it
	kvd_ptr = kvd_iArray;

	// whilst treating kvd_ptr as any other pointer
	printf("\n\n");
	printf("accessing kvd_iArray[] using the alias kvd_ptr:\n\n");
	for (kvd_i = 0; kvd_i < sizeof(kvd_iArray) / sizeof(int); kvd_i++)
		printf("\t*(kvd_ptr + %d) = %d; at address = 0x%p\n", kvd_i, *(kvd_iArray + kvd_i), (kvd_iArray + kvd_i));
	
	printf("\n\n");
	return(0);
}
