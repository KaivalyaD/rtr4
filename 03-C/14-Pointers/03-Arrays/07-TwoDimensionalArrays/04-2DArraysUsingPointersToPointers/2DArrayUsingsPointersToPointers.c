#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	// variable declarations
	int **kvd_i2dArray = NULL, kvd_i, kvd_j, kvd_row_count, kvd_col_count;

	// code
	printf("\n\n");
	printf("enter the number of rows: ");
	scanf("%d", &kvd_row_count);
	printf("enter the number of columns: ");
	scanf("%d", &kvd_col_count);

	printf("\n\n");

	// allocating memory to kvd_iArray
	kvd_i2dArray = (int **)malloc(sizeof(int *) * kvd_row_count);
	if (!kvd_i2dArray)
	{
		printf("malloc(): failed to allocate %d integer pointers\n\n", kvd_row_count);
		return 1;
	}
	for (kvd_i = 0; kvd_i < kvd_row_count; kvd_i++)
	{
		*(kvd_i2dArray + kvd_i) = (int *)malloc(sizeof(int) * kvd_col_count);
		if (!(*(kvd_i2dArray + kvd_i)))
		{
			printf("malloc(): failed to allocate %d integers in row %d\n\n", kvd_col_count, kvd_i + 1);
			return 1;
		}
	}
	printf("memory allocated successfully\n\n");

	// initializing kvd_iArray
	for (kvd_i = 0; kvd_i < kvd_row_count; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < kvd_col_count; kvd_j++)
			*(*(kvd_i2dArray + kvd_i) + kvd_j) = (kvd_j + 2) * (kvd_i + 1);
	}

	// displaying kvd_i2dArray
	printf("elements and their addresses in kvd_i2dArray:\n\n");
	for (kvd_i = 0; kvd_i < kvd_row_count; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < kvd_col_count; kvd_j++)
			printf("\tkvd_i2dArray[%d][%d] = %d; at address = 0x%p\n", kvd_i, kvd_j, *(*(kvd_i2dArray + kvd_i) + kvd_j), *(kvd_i2dArray + kvd_i) + kvd_j);
		printf("\n\n");
	}

	// freeing all allocated memory
	for (kvd_i = kvd_row_count - 1; kvd_i >= 0; kvd_i--)
	{
		if (*(kvd_i2dArray + kvd_i))
		{
			free(*(kvd_i2dArray + kvd_i));
			*(kvd_i2dArray + kvd_i) = NULL;

			printf("freed and cleaned the memory block pointed to by kvd_i2dArray[%d]\n", kvd_i);
		}
	}
	if (kvd_i2dArray)
	{
		free(kvd_i2dArray);
		kvd_i2dArray = NULL;

		printf("freed and cleaned all dynamically allocated memory\n\n");
	}

	return 0;
}
