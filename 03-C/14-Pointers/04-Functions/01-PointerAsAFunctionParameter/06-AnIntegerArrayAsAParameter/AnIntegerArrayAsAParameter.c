#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	// function prototypes
	void ElementWiseProduct(int *, int, int);

	// variable declarations
	int *kvd_iArray = NULL, kvd_element_count, kvd_i, kvd_n;

	// code
	printf("\n\n");
	printf("enter the number of elements you want in the array: ");
	scanf("%d", &kvd_element_count);

	kvd_iArray = (int *)malloc(sizeof(int) * kvd_element_count);
	if (!kvd_iArray)
	{
		printf("\n\n");
		printf("malloc(): failed to allocate memory for %d integers\n\n", kvd_element_count);
		return 1;
	}

	printf("\n\n");
	printf("populate the array with %d integers:\n\n", kvd_element_count);
	for (kvd_i = 0; kvd_i < kvd_element_count; kvd_i++)
	{
		printf("\t");
		scanf("%d", kvd_iArray + kvd_i);
	}
	printf("\n\n");
	
	printf("you entered:\n\n");
	for (kvd_i = 0; kvd_i < kvd_element_count; kvd_i++)
		printf("\tkvd_iArray[%d] = %d\n", kvd_i, *(kvd_iArray + kvd_i));
	printf("\n\n");
	
	printf("enter the number with which to multiply each element in the array: ");
	scanf("%d", &kvd_n);

	ElementWiseProduct(kvd_iArray, kvd_element_count, kvd_n);

	printf("\n\n");
	printf("results:\n\n");
	for (kvd_i = 0; kvd_i < kvd_element_count; kvd_i++)
		printf("\tkvd_iArray[%d] = %d\n", kvd_i, *(kvd_iArray + kvd_i));
	printf("\n\n");

	if (kvd_iArray)
	{
		free(kvd_iArray);
		kvd_iArray = NULL;

		printf("freed and cleaned kvd_iArray\n\n");
	}

	return 0;
}

void ElementWiseProduct(int *kvd_iArray, int kvd_element_count, int kvd_n)
{
	// variable declarations
	int kvd_i;

	// code
	for (kvd_i = 0; kvd_i < kvd_element_count; kvd_i++)
		*(kvd_iArray + kvd_i) = *(kvd_iArray + kvd_i) * kvd_n;
}
