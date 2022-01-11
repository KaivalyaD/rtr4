#include <stdio.h>
#include <stdlib.h>

#define COUNT_ROWS 5
#define COUNT_COLS 3

int main(void)
{
	// variable declarations
	int **kvd_pi2dArray = NULL, kvd_i, kvd_j;

	// code
	printf("\n\n");

	// first, allocate memory for COUNT_ROWS arrays of type int *
	kvd_pi2dArray = (int **)malloc(sizeof(int *) * COUNT_ROWS);
	if (!kvd_pi2dArray)
	{
		printf("\n\n");
		printf("malloc(): failed to allocate memory for %d integer pointers\n\n", COUNT_ROWS);
		return 1;
	}
	// next, for each row, allocate memory for COUNT_COLS columns of type int
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		*(kvd_pi2dArray + kvd_i) = (int *)malloc(sizeof(int) * COUNT_COLS);
		if (!(*(kvd_pi2dArray + kvd_i)))
		{
			printf("\n\n");
			printf("malloc(): failed to allocate memory for %d integers for row %d\n", COUNT_COLS, kvd_i + 1);
			return 1;
		}
	}
	printf("memory allocated successfully\n\n");
	
	// intializing kvd_pi2dArray
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < COUNT_COLS; kvd_j++)
			*(*(kvd_pi2dArray + kvd_i) + kvd_j) = (kvd_j + 2) * (kvd_i + 1);
	}

	// displaying values
	printf("elements and their addresses in kvd_pi2dArray[][]:\n\n");
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < COUNT_COLS; kvd_j++)
			printf("\t*(*(kvd_pi2dArray + %d) + %d) = %d; at address = 0x%p\n", kvd_i, kvd_j, *(*(kvd_pi2dArray + kvd_i) + kvd_j), *(kvd_pi2dArray + kvd_i) + kvd_j);
		printf("\n\n");
	}

	// freeing allocated memory in the order of destructor
	printf("\n\n");
	for (kvd_i = COUNT_ROWS - 1; kvd_i >= 0; kvd_i--)
	{
		if (*(kvd_pi2dArray + kvd_i))
		{
			free(*(kvd_pi2dArray + kvd_i));
			*(kvd_pi2dArray + kvd_i) = NULL;
			
			printf("freed and cleaned memory occupied by row %d\n", kvd_i + 1);
		}
	}
	if (kvd_pi2dArray)
	{
		free(kvd_pi2dArray);
		kvd_pi2dArray = NULL;
		
		printf("freed and cleaned all dynamically allocated memory\n\n");
	}

	return 0;
}
