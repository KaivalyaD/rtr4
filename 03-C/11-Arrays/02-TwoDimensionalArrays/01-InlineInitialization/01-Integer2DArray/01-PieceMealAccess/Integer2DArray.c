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
	int kvd_int_size, kvd_iArray_size, kvd_iArray_row_count, kvd_iArray_col_count, kvd_iArray_element_count;

	// code
	printf("\n\n");

	kvd_int_size = sizeof(int);
	
	kvd_iArray_size = sizeof(kvd_iArray);
	printf("size of the 2D array, kvd_iArray[][] = %d\n", kvd_iArray_size);

	kvd_iArray_row_count = kvd_iArray_size / sizeof(kvd_iArray[0]);
	printf("number of rows in kvd_iArray[][] = %d\n", kvd_iArray_row_count);

	kvd_iArray_col_count = sizeof(kvd_iArray[0]) / kvd_int_size;
	printf("number of columns in kvd_iArray[][] = %d\n", kvd_iArray_col_count);

	kvd_iArray_element_count = kvd_iArray_row_count * kvd_iArray_col_count;
	printf("number of elements in kvd_iArray[][] = %d\n", kvd_iArray_element_count);

	printf("\n\n");

	printf("elements in the 2D array, kvd_iArray[][] are:\n\n");
	
	// row 1
	printf("***** ROW 1 *****\n");
	printf("\tkvd_iArray[0][0] = %d\n", kvd_iArray[0][0]);  // column 1 
	printf("\tkvd_iArray[0][1] = %d\n", kvd_iArray[0][1]);  // column 2
	printf("\tkvd_iArray[0][2] = %d\n", kvd_iArray[0][2]);  // column 3

	printf("\n");
	
	// row 2
	printf("***** ROW 2 *****\n");
	printf("\tkvd_iArray[1][0] = %d\n", kvd_iArray[1][0]);  // column 1
	printf("\tkvd_iArray[1][1] = %d\n", kvd_iArray[1][1]);  // column 2
	printf("\tkvd_iArray[1][2] = %d\n", kvd_iArray[1][2]);  // column 3

	printf("\n");

	// row 3
	printf("***** ROW 3 *****\n");
	printf("\tkvd_iArray[2][0] = %d\n", kvd_iArray[2][0]);  // column 1
	printf("\tkvd_iArray[2][1] = %d\n", kvd_iArray[2][1]);  // column 2
	printf("\tkvd_iArray[2][2] = %d\n", kvd_iArray[2][2]);  // column 3

	printf("\n");

	// row 4
	printf("***** ROW 4 *****\n");
	printf("\tkvd_iArray[3][0] = %d\n", kvd_iArray[3][0]);  // column 1
	printf("\tkvd_iArray[3][1] = %d\n", kvd_iArray[3][1]);  // column 2
	printf("\tkvd_iArray[3][2] = %d\n", kvd_iArray[3][2]);  // column 3

	printf("\n");

	// row 5
	printf("***** ROW 5 *****\n");
	printf("\tkvd_iArray[4][0] = %d\n", kvd_iArray[4][0]);  // column 1
	printf("\tkvd_iArray[4][1] = %d\n", kvd_iArray[4][1]);  // column 2
	printf("\tkvd_iArray[4][2] = %d\n", kvd_iArray[4][2]);  // column 3

	printf("\n\n");
	return(0);
}
