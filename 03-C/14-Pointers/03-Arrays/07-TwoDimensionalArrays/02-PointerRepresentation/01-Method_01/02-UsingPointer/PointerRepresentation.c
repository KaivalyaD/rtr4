#include <stdio.h>

#define COUNT_ROWS 5
#define COUNT_COLS 3

int main(void)
{
	// variable declarations
	int kvd_iArray[COUNT_ROWS][COUNT_COLS], kvd_i, kvd_j;
	int *kvd_piArray_current_row = NULL;

	// code
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		// getting the base address of each 1D array
		kvd_piArray_current_row = kvd_iArray[kvd_i];

		for (kvd_j = 0; kvd_j < COUNT_COLS; kvd_j++)
			*(kvd_piArray_current_row + kvd_j) = (kvd_i + 2) * (kvd_j + 1);
	}

	printf("\n\n");

	printf("elements and their addresses in kvd_iArray[][]:\n\n");
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		kvd_piArray_current_row = *(kvd_iArray + kvd_i);

		for (kvd_j = 0; kvd_j < COUNT_COLS; kvd_j++)
			printf("\tkvd_iArray[%d][%d] = %d; at address = 0x%p\n", kvd_i, kvd_j, *(kvd_piArray_current_row + kvd_j), (kvd_piArray_current_row + kvd_j));
		
		printf("\n\n");
	}

	return 0;
}
