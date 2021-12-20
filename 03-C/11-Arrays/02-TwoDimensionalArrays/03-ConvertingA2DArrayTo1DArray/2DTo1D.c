#include <stdio.h>

#define KVD_ROW_COUNT 5
#define KVD_COLUMN_COUNT 3

int main(void)
{
	// variable declarations
	int kvd_iArray_2D[KVD_ROW_COUNT][KVD_COLUMN_COUNT];
	int kvd_iArray_1D[KVD_ROW_COUNT * KVD_COLUMN_COUNT];
	int kvd_i, kvd_j, kvd_num;

	// code
	printf("\n\n");

	// populate the 2D array
	for (kvd_i = 0; kvd_i < KVD_ROW_COUNT; kvd_i++)
	{
		printf("for row %d:\n\n", kvd_i + 1);
		for (kvd_j = 0; kvd_j < KVD_COLUMN_COUNT; kvd_j++)
		{
			printf("\tenter element number %d: ", (kvd_j + 1));
			scanf("%d", &kvd_num);
			kvd_iArray_2D[kvd_i][kvd_j] = kvd_num;
		}
		printf("\n\n");
	}

	// display the populated 2D array
	printf("you entered:\n\n");
	for (kvd_i = 0; kvd_i < KVD_ROW_COUNT; kvd_i++)
	{
		printf("\t***** row %d *****\n\n", kvd_i + 1);
		for (kvd_j = 0; kvd_j < KVD_COLUMN_COUNT; kvd_j++)
			printf("\t\telement %d = %d\n", kvd_j + 1, kvd_iArray_2D[kvd_i][kvd_j]);
		printf("\n\n");
	}

	// down convert
	for (kvd_i = 0; kvd_i < KVD_ROW_COUNT; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < KVD_COLUMN_COUNT; kvd_j++)
			kvd_iArray_1D[(kvd_i * KVD_COLUMN_COUNT) + kvd_j] = kvd_iArray_2D[kvd_i][kvd_j];
	}

	// display the 1D array
	printf("converted 1D array:\n\n");
	for (kvd_i = 0; kvd_i < (KVD_ROW_COUNT * KVD_COLUMN_COUNT); kvd_i++)
		printf("\telement %d = %d\n", kvd_i + 1, kvd_iArray_1D[kvd_i]);

	printf("\n\n");
	return(0);
}
