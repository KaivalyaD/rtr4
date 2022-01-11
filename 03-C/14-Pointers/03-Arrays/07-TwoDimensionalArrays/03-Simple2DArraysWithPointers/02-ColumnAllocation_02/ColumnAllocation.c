#include <stdio.h>
#include <stdlib.h>

#define COUNT_ROWS 5
#define COUNT_COLS1 3
#define COUNT_COLS2 8

int main(void)
{
	// variable declarations
	int *kvd_iArray[COUNT_ROWS], kvd_i, kvd_j;

	// code
	printf("\n\n");

	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		kvd_iArray[kvd_i] = (int *)malloc(sizeof(int) * COUNT_COLS1);
		if (!kvd_iArray[kvd_i])
		{
			printf("malloc(): failed to allocate memory for %d integers in row %d\n", COUNT_COLS1, kvd_i);
			return 1;
		}
	}
	printf("successfully allocated memory\n\n");

	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < COUNT_COLS1; kvd_j++)
			kvd_iArray[kvd_i][kvd_j] = (kvd_j + 2) * (kvd_i + 1);
	}

	printf("elements and their addresses in kvd_iArray[][]:\n\n");
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < COUNT_COLS1; kvd_j++)
			printf("\tkvd_iArray[%d][%d] = %d; at address = 0x%p\n", kvd_i, kvd_j, kvd_iArray[kvd_i][kvd_j], &kvd_iArray[kvd_i][kvd_j]);
		printf("\n\n");
	}
	
	for (kvd_i = COUNT_ROWS - 1; kvd_i >= 0; kvd_i--)
	{
		if (kvd_iArray[kvd_i])
		{
			free(kvd_iArray[kvd_i]);
			kvd_iArray[kvd_i] = NULL;

			printf("freed and cleaned memory block pointed to by kvd_iArray[%d]\n", kvd_i);
		}
	}
	printf("freed all dynamically allocated memory\n\n");

	// allocating memory once again for each row in kvd_iArray, this time
	// with COUNT_COLS2 columns per row

	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		kvd_iArray[kvd_i] = (int *)malloc(sizeof(int) * COUNT_COLS2);
		if (!kvd_iArray[kvd_i])
		{
			printf("malloc(): failed to allocate memory for %d integers in row %d\n\n", COUNT_COLS2, kvd_i);
			return 1;
		}
	}
	printf("successfully allocated memory again\n\n");

	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < COUNT_COLS2; kvd_j++)
			kvd_iArray[kvd_i][kvd_j] = (kvd_j + 2) * (kvd_i + 2);
	}

	printf("elements and their addresses in the new kvd_iArray[][]:\n\n");
	for (kvd_i = 0; kvd_i < COUNT_ROWS; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < COUNT_COLS2; kvd_j++)
			printf("\tkvd_iArray[%d][%d] = %d; at address = 0x%p\n", kvd_i, kvd_j, kvd_iArray[kvd_i][kvd_j], &kvd_iArray[kvd_i][kvd_j]);
		printf("\n\n");
	}

	for (kvd_i = COUNT_ROWS - 1; kvd_i >= 0; kvd_i--)
	{
		if (kvd_iArray[kvd_i])
		{
			free(kvd_iArray[kvd_i]);
			kvd_iArray[kvd_i] = NULL;

			printf("freed and cleaned memory block pointed to by kvd_iArray[%d]\n", kvd_i);
		}
	}
	printf("freed all dynamically allocated memory again\n\n");

	return 0;
}
