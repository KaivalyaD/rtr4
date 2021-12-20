#include <stdio.h>

#define KVD_WIDTH 2
#define KVD_HEIGHT 3
#define KVD_DEPTH 5

int main(void)
{
	// variable declarations
	int kvd_iArray_3D[KVD_DEPTH][KVD_HEIGHT][KVD_WIDTH] = {
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
	};
	int kvd_iArray_1D[KVD_WIDTH * KVD_HEIGHT * KVD_DEPTH];
	int kvd_i, kvd_j, kvd_k;

	// code
	printf("\n\n");

	// display the 3D array
	printf("elements in the 3D array, kvd_iArray[][][]:\n\n");
	for (kvd_i = 0; kvd_i < KVD_DEPTH; kvd_i++)
	{
		printf("***** Plane %d *****\n\n", kvd_i + 1);
		for (kvd_j = 0; kvd_j < KVD_HEIGHT; kvd_j++)
		{
			printf("\t***** Row %d *****\n\n", kvd_j + 1);
			for (kvd_k = 0; kvd_k < KVD_WIDTH; kvd_k++)
				printf("\t\tkvd_iArray_3D[%d][%d][%d] = %d\n", kvd_i, kvd_j, kvd_k, kvd_iArray_3D[kvd_i][kvd_j][kvd_k]);
			printf("\n");
		}
		printf("\n\n");
	}

	// down convert
	for (kvd_i = 0; kvd_i < KVD_DEPTH; kvd_i++)
	{
		for (kvd_j = 0; kvd_j < KVD_HEIGHT; kvd_j++)
		{
			for (kvd_k = 0; kvd_k < KVD_DEPTH; kvd_k++)
			{
				kvd_iArray_1D[(kvd_i * KVD_WIDTH  * KVD_HEIGHT) + (kvd_j * KVD_WIDTH) + kvd_k] = kvd_iArray_3D[kvd_i][kvd_j][kvd_k];
			}
		}
	}

	// display the converted 1D array
	printf("down converted 1D array:\n\n");
	for (kvd_i = 0; kvd_i < (KVD_WIDTH * KVD_HEIGHT * KVD_DEPTH); kvd_i++)
		printf("\tkvd_iArray_1D[%d] = %d\n", kvd_i, kvd_iArray_1D[kvd_i]);

	printf("\n\n");
	return(0);
}
