#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_iArray[5][3][2] = {
		{
			{9, 18},
			{27, 36},
			{45, 54}
		},
		{
			{8, 16},
			{24, 32},
			{40, 48}
		},
		{
			{7, 14},
			{21, 28},
			{35, 42}
		},
		{
			{6, 12},
			{18, 24},
			{30, 36}
		},
		{
			{5, 10},
			{15, 20},
			{25, 30}
		}
	};  // inline initialization
	int kvd_iArray_size, kvd_iArray_width, kvd_iArray_height, kvd_iArray_depth, kvd_element_count;
	int kvd_i, kvd_j, kvd_k;

	// code
	printf("\n\n");

	kvd_iArray_size = sizeof(kvd_iArray);
	printf("size of the 3D array, kvd_iArray[][][] = %d\n", kvd_iArray_size);

	kvd_iArray_width = sizeof(kvd_iArray[0][0]) / sizeof(kvd_iArray[0][0][0]);
	printf("width of kvd_iArray[][][] = %d\n", kvd_iArray_width);

	kvd_iArray_height = sizeof(kvd_iArray[0]) / sizeof(kvd_iArray[0][0]);
	printf("height of kvd_iArray[][][] = %d\n", kvd_iArray_height);

	kvd_iArray_depth = kvd_iArray_size / sizeof(kvd_iArray[0]);
	printf("depth of kvd_iArray[][][] = %d\n", kvd_iArray_depth);

	kvd_element_count = kvd_iArray_width * kvd_iArray_height * kvd_iArray_depth;
	printf("number of elements in the 3D array, kvd_iArray[][][] = %d\n", kvd_element_count);

	printf("\n\n");

	printf("elements in kvd_iArray[][][]:\n\n");
	for (kvd_i = 0; kvd_i < kvd_iArray_depth; kvd_i++)
	{
		printf("***** Plane %d *****\n\n", kvd_i + 1);
		for (kvd_j = 0; kvd_j < kvd_iArray_height; kvd_j++)
		{
			printf("\t***** Row %d ******\n\n", kvd_j + 1);
			for (kvd_k = 0; kvd_k < kvd_iArray_width; kvd_k++)
				printf("\t\tkvd_iArray[%d][%d][%d] = %d\n", kvd_i, kvd_j, kvd_k, kvd_iArray[kvd_i][kvd_j][kvd_k]);
			printf("\n");
		}
		printf("\n\n");
	}

	return(0);
}
