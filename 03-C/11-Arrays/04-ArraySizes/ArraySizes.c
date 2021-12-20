#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray_One[5];
	int kvd_iArray_Two[5][3];
	int kvd_iArray_Three[100][100][5];

	int kvd_2D_row_count, kvd_2D_column_count;
	int kvd_3D_width, kvd_3D_height, kvd_3D_depth;

	// code
	printf("\n\n");

	printf("size of 1D integer array, kvd_iArray_One[] = %zd\n", sizeof(kvd_iArray_One));
	printf("number of elements in kvd_iArray_One[] = %lld\n\n", sizeof(kvd_iArray_One) / sizeof(int));

	kvd_2D_column_count = sizeof(kvd_iArray_Two[0]) / sizeof(kvd_iArray_Two[0][0]);
	kvd_2D_row_count = sizeof(kvd_iArray_Two) / sizeof(kvd_iArray_Two[0]);
	printf("size of 2D integer array, kvd_iArray_Two[][] = %zd\n", sizeof(kvd_iArray_Two));
	printf("number of rows in kvd_iArray_Two[][] = %d\n", kvd_2D_row_count);
	printf("number of columns in kvd_iArray_Two[][] = %d\n", kvd_2D_column_count);
	printf("number of elements in kvd_iArray_Two[][] = %d\n\n", kvd_2D_row_count * kvd_2D_column_count);

	kvd_3D_width = sizeof(kvd_iArray_Three[0][0]) / sizeof(kvd_iArray_Three[0][0][0]);
	kvd_3D_height = sizeof(kvd_iArray_Three[0]) / sizeof(kvd_iArray_Three[0][0]);
	kvd_3D_depth = sizeof(kvd_iArray_Three) / sizeof(kvd_iArray_Three[0]);
	printf("size of 3D integer array, kvd_iArray_Three[][][] = %zd\n", sizeof(kvd_iArray_Three));
	printf("width of kvd_iArray_Three[][][] = %d\n", kvd_3D_width);
	printf("height of kvd_iArray_Three[][][] = %d\n", kvd_3D_height);
	printf("depth of kvd_iArray_Three[][][] = %d\n", kvd_3D_depth);
	printf("number of elements in kvd_iArray_Three[][][] = %d\n\n", kvd_3D_width * kvd_3D_height * kvd_3D_depth);

	return(0);
}
