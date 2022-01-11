#include <stdio.h>
#include <stdlib.h>

#define COUNT_ROWS 5
#define COUNT_COLS 5

int main(void)
{
	// variable declarations
	int *kvd_iArray[COUNT_ROWS], kvd_i, kvd_j;

	// code
	printf("\n\n");

	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		// allocating (COUNT_ROWS - kvd_i) integers per row, i.e.
		//		row 0 - 5 columns
		//		row 1 - 4 columns
		//		row 2 - 3 columns
		//		row 3 - 2 columns
		//		row 4 - 1 column
		// 
		// power of pointers!
		kvd_iArray[kvd_i] = (int *)malloc(sizeof(int) * (COUNT_COLS - kvd_i));
		if (!kvd_iArray[kvd_i])
		{
			printf("malloc(): failed to allocate memory for %d integers in row %d\n\n", (COUNT_COLS - kvd_i), kvd_i);
			return 1;
		}
	}
	printf("successfully allocated memory\n\n");

	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < (COUNT_ROWS - kvd_i); kvd_j++)
			kvd_iArray[kvd_i][kvd_j] = (kvd_j + 2) * (kvd_i + 1);
	}

	printf("elements and their addresses in kvd_iArray[][]:\n\n");
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < (COUNT_COLS - kvd_i); kvd_j++)
			printf("\tkvd_iArray[%d][%d] = %d; at address = 0x%p\n", kvd_i, kvd_j, kvd_iArray[kvd_i][kvd_j], &kvd_iArray[kvd_i][kvd_j]);
		printf("\n\n");
	}

	for (kvd_i = COUNT_ROWS - 1; kvd_i >= 0; kvd_i--)
	{
		if (kvd_iArray[kvd_i])
		{
			free(kvd_iArray[kvd_i]);
			kvd_iArray[kvd_i] = NULL;

			printf("freed and cleaned the memory block pointed to by kvd_iArray[%d]\n", kvd_i);
		}
	}
	printf("freed and cleaned all dynamically allocated memory\n\n");

	return 0;
}
