#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[5][3][2] = {
		{  // plane 1
			{9, 18},   // row 1 {column 1, column 2}
			{27, 36},  // row 2 {column 1, column 2}
			{45, 54}   // row 3 {column 1, column 2}
		},
		{  // plane 2
			{8, 16},   // row 1 {column 1, column 2}
			{24, 32},  // row 2 {column 1, column 2}
			{40, 48}   // row 3 {column 1, column 2}
		},
		{  // plane 3
			{7, 14},   // row 1 {column 1, column 2}
			{21, 28},  // row 2 {column 1, column 2}
			{35, 42}   // row 3 {column 1, column 2}
		},
		{  // plane 4
			{6, 12},   // row 1 {column 1, column 2}
			{18, 24},  // row 2 {column 1, column 2}
			{30, 36}   // row 3 {column 1, column 2}
		},
		{  // plane 5
			{5, 10},   // row 1 {column 1, column 2}
			{15, 20},  // row 2 {column 1, column 2}
			{25, 30}   // row 3 {column 1, column 2}
		}
	};  // Inline initialization
	int kvd_iArray_size, kvd_iArray_plane_count, kvd_iArray_row_count, kvd_iArray_col_count, kvd_iArray_element_count;

	// code
	printf("\n\n");

	kvd_iArray_size = sizeof(kvd_iArray);
	printf("size of the 3D array, kvd_iArray[][][] = %d\n", kvd_iArray_size);

	kvd_iArray_col_count = sizeof(kvd_iArray[0][0]) / sizeof(kvd_iArray[0][0][0]);  // also called the 'width'
	printf("number of columns per row (width) = %d\n", kvd_iArray_col_count);

	kvd_iArray_row_count = sizeof(kvd_iArray[0]) / sizeof(kvd_iArray[0][0]);  // also called the 'height'
	printf("number of rows per plane (height) = %d\n", kvd_iArray_row_count);

	kvd_iArray_plane_count = kvd_iArray_size / sizeof(kvd_iArray[0]);  // also called the 'depth'
	printf("number of planes in kvd_iArray[][][] (depth) = %d\n", kvd_iArray_plane_count);

	kvd_iArray_element_count = kvd_iArray_col_count * kvd_iArray_row_count * kvd_iArray_plane_count;
	printf("number of elements in kvd_iArray[][][] = %d\n", kvd_iArray_element_count);

	printf("\n\n");

	// piece-meal display
	printf("***** Plane 1 *****\n\n");

	printf("\t***** Row 1 *****\n\n");
	printf("\t\tkvd_iArray[0][0][0] = %d\n", kvd_iArray[0][0][0]);
	printf("\t\tkvd_iArray[0][0][1] = %d\n\n", kvd_iArray[0][0][1]);
	
	printf("\t***** Row 2 *****\n\n");
	printf("\t\tkvd_iArray[0][1][0] = %d\n", kvd_iArray[0][1][0]);
	printf("\t\tkvd_iArray[0][1][1] = %d\n\n", kvd_iArray[0][1][1]);

	printf("\t***** Row 3 *****\n\n");
	printf("\t\tkvd_iArray[0][2][0] = %d\n", kvd_iArray[0][2][0]);
	printf("\t\tkvd_iArray[0][2][1] = %d\n\n", kvd_iArray[0][2][1]);

	printf("***** Plane 2 *****\n\n");

	printf("\t***** Row 1 *****\n\n");
	printf("\t\tkvd_iArray[1][0][0] = %d\n", kvd_iArray[1][0][0]);
	printf("\t\tkvd_iArray[1][0][1] = %d\n\n", kvd_iArray[1][0][1]);
	
	printf("\t***** Row 2 *****\n\n");
	printf("\t\tkvd_iArray[1][1][0] = %d\n", kvd_iArray[1][1][0]);
	printf("\t\tkvd_iArray[1][1][1] = %d\n\n", kvd_iArray[1][1][1]);

	printf("\t***** Row 3 *****\n\n");
	printf("\t\tkvd_iArray[1][2][0] = %d\n", kvd_iArray[1][2][0]);
	printf("\t\tkvd_iArray[1][2][1] = %d\n\n", kvd_iArray[1][2][1]);

	printf("***** Plane 3 *****\n\n");
	
	printf("\t***** Row 1 *****\n\n");
	printf("\t\tkvd_iArray[2][0][0] = %d\n", kvd_iArray[2][0][0]);
	printf("\t\tkvd_iArray[2][0][1] = %d\n\n", kvd_iArray[2][0][1]);

	printf("\t***** Row 2 *****\n\n");
	printf("\t\tkvd_iArray[2][1][0] = %d\n", kvd_iArray[2][1][0]);
	printf("\t\tkvd_iArray[2][1][1] = %d\n\n", kvd_iArray[2][1][1]);

	printf("\t***** Row 3 *****\n\n");
	printf("\t\tkvd_iArray[2][2][0] = %d\n", kvd_iArray[2][2][0]);
	printf("\t\tkvd_iArray[2][2][1] = %d\n\n", kvd_iArray[2][2][1]);

	printf("***** Plane 4 *****\n\n");

	printf("\t***** Row 1 *****\n\n");
	printf("\t\tkvd_iArray[3][0][0] = %d\n", kvd_iArray[3][0][0]);
	printf("\t\tkvd_iArray[3][0][1] = %d\n\n", kvd_iArray[3][0][1]);

	printf("\t***** Row 2 *****\n\n");
	printf("\t\tkvd_iArray[3][1][0] = %d\n", kvd_iArray[3][1][0]);
	printf("\t\tkvd_iArray[3][1][1] = %d\n\n", kvd_iArray[3][1][1]);

	printf("\t***** Row 3 *****\n\n");
	printf("\t\tkvd_iArray[3][2][0] = %d\n", kvd_iArray[3][2][0]);
	printf("\t\tkvd_iArray[3][2][1] = %d\n\n", kvd_iArray[3][2][1]);

	printf("***** Plane 5 *****\n\n");

	printf("\t***** Row 1 *****\n\n");
	printf("\t\tkvd_iArray[4][0][0] = %d\n", kvd_iArray[4][0][0]);
	printf("\t\tkvd_iArray[4][0][1] = %d\n\n", kvd_iArray[4][0][1]);

	printf("\t***** Row 2 *****\n\n");
	printf("\t\tkvd_iArray[4][1][0] = %d\n", kvd_iArray[4][1][0]);
	printf("\t\tkvd_iArray[4][1][1] = %d\n\n", kvd_iArray[4][1][1]);

	printf("\t***** Row 3 *****\n\n");
	printf("\t\tkvd_iArray[4][2][0] = %d\n", kvd_iArray[4][2][0]);
	printf("\t\tkvd_iArray[4][2][1] = %d\n\n", kvd_iArray[4][2][1]);

	return(0);
}
