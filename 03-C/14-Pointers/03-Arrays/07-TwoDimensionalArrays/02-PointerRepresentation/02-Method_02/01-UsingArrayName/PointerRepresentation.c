#include <stdio.h>

#define COUNT_ROWS 5
#define COUNT_COLS 3

int main(void)
{
	// variable declarations
	int kvd_iArray[COUNT_ROWS][COUNT_COLS], kvd_i, kvd_j;

	// code
	// kvd_iArray[][]       -> the base address of a 2D array
	// kvd_iArray[i]        -> the base address of the ith row
	// kvd_iArray[i] + j    -> address of the jth element in the ith row
	// *(kvd_iArray[i] + j) -> value at address of the jth element in the ith row => kvd_iArray[0][0]
	// *(*(kvd_iArray + i) + j) -> same description as above

	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < COUNT_COLS; kvd_j++)
			*(kvd_iArray[kvd_i] + kvd_j) = (kvd_j + 2) * (kvd_i + 1);
	}

	printf("\n\n");
	printf("elements and their addresses in kvd_iArray[][]:\n\n");
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < COUNT_COLS; kvd_j++)
			printf("\tkvd_iArray[%d][%d] = %d; at address = 0x%p\n", kvd_i, kvd_j, *(*(kvd_iArray + kvd_i) + kvd_j), (*(kvd_iArray + kvd_i) + kvd_j));
		printf("\n\n");
	}

	return 0;
}
