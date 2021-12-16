#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[5][3] = {
		{1, 2, 3},
		{2, 4, 6},
		{3, 6, 9},
		{4, 8, 12},
		{5, 10, 15}
	};  // inline initialization
	int kvd_iArray_size, kvd_iArray_row_count, kvd_iArray_col_count, kvd_iArray_element_count;
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	kvd_iArray_size = sizeof(kvd_iArray);
	printf("size of the 2D array, kvd_iArray[][] = %d\n", kvd_iArray_size);

	kvd_iArray_row_count = kvd_iArray_size / sizeof(kvd_iArray[0]);
	printf("number of rows in kvd_iArray[][] = %d\n", kvd_iArray_row_count);

	kvd_iArray_col_count = sizeof(kvd_iArray[0]) / sizeof(kvd_iArray[0][0]);
	printf("number of columns in kvd_iArray[][] = %d\n", kvd_iArray_col_count);

	kvd_iArray_element_count = kvd_iArray_row_count * kvd_iArray_col_count;
	printf("number of elements in kvd_iArray[][] = %d\n", kvd_iArray_element_count);

	printf("\n\n");

	printf("elements in the 2D array, kvd_iArray[][]:\n\n");
	for (kvd_i = 0; kvd_i < kvd_iArray_row_count; kvd_i++)
	{
		printf("***** ROW %d *****\n", kvd_i+1);
		
		for (kvd_j = 0; kvd_j < kvd_iArray_col_count; kvd_j++)
			printf("\tArray[%d][%d] = %d\n", kvd_i, kvd_j, kvd_iArray[kvd_i][kvd_j]);
		
		printf("\n\n");
	}

	return(0);
}
