#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[3][5];  // 3 rows, 5 columns
	int kvd_iArray_size, kvd_iArray_element_count, kvd_iArray_row_count, kvd_iArray_col_count;
	int kvd_i, kvd_j;
	
	// code
	printf("\n\n");

	kvd_iArray_size = sizeof(kvd_iArray);
	printf("size of the 2D integer array, kvd_iArray[][] = %d\n", kvd_iArray_size);

	kvd_iArray_row_count = kvd_iArray_size / sizeof(kvd_iArray[0]);
	printf("number of rows in kvd_iArray[][] = %d\n", kvd_iArray_row_count);

	kvd_iArray_col_count = sizeof(kvd_iArray[0]) / sizeof(kvd_iArray[0][0]);
	printf("number of columns in kvd_iArray[][] = %d\n", kvd_iArray_col_count);

	kvd_iArray_element_count = kvd_iArray_row_count * kvd_iArray_col_count;
	printf("number of elements in kvd_iArray[][] = %d\n", kvd_iArray_element_count);

	/* piece-meal assignment */
	// ***** row 1 *****
	kvd_iArray[0][0] = 21;
	kvd_iArray[0][1] = 42;
	kvd_iArray[0][2] = 63;
	kvd_iArray[0][3] = 84;
	kvd_iArray[0][4] = 105;

	// ***** row 2 *****
	kvd_iArray[1][0] = 22;
	kvd_iArray[1][1] = 44;
	kvd_iArray[1][2] = 66;
	kvd_iArray[1][3] = 88;
	kvd_iArray[1][4] = 110;

	// ***** row 3 *****
	kvd_iArray[2][0] = 23;
	kvd_iArray[2][1] = 46;
	kvd_iArray[2][2] = 69;
	kvd_iArray[2][3] = 92;
	kvd_iArray[2][4] = 115;

	/* displaying the array */
	printf("\n\n");

	printf("elements in kvd_iArray[][]:\n\n");
	for (kvd_i = 0; kvd_i < kvd_iArray_row_count; kvd_i++)
	{
		printf("***** ROW %d *****\n\n", kvd_i);

		for (kvd_j = 0; kvd_j < kvd_iArray_col_count; kvd_j++)
			printf("\tkvd_iArray[%d][%d] = %d\n", kvd_i, kvd_j, kvd_iArray[kvd_i][kvd_j]);

		printf("\n\n");
	}

	return(0);
}
