#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	// variable declarations
	int *kvd_piArray = NULL, kvd_i;
	unsigned int kvd_iArray_length = 0;

	float *kvd_pfArray = NULL;
	unsigned int kvd_fArray_length = 0;

	double *kvd_pdArray = NULL;
	unsigned int kvd_dArray_length = 0;

	char *kvd_pcArray = NULL;
	unsigned int kvd_cArray_length = 0;

	// code
	printf("\n\n");

	// the integer array
	printf("enter the number of elements in the integer array: ");
	scanf("%u", &kvd_iArray_length);

	kvd_piArray = (int *)malloc(sizeof(int) * kvd_iArray_length);
	if (!kvd_piArray)
	{
		printf("\n\n");
		printf("malloc(): failed to allocate memory for %d integers\n\n", kvd_iArray_length);
		return 1;
	}
	printf("memory allocation succeeded\n\n");

	printf("enter %d integers:\n\n", kvd_iArray_length);
	for (kvd_i = 0; kvd_i < kvd_iArray_length; kvd_i++)
	{
		printf("\t");
		scanf("%d", kvd_piArray + kvd_i);
	}

	// the float array
	printf("\n\n");
	printf("enter the number of elements in the float array: ");
	scanf("%u", &kvd_fArray_length);

	kvd_pfArray = (float *)malloc(sizeof(float) * kvd_fArray_length);
	if (!kvd_pfArray)
	{
		printf("\n\n");
		printf("malloc(): failed to allocate memory for %d floats\n\n", kvd_fArray_length);
		return 1;
	}
	printf("memory allocation succeeded\n\n");

	printf("enter %d float values:\n\n", kvd_fArray_length);
	for (kvd_i = 0; kvd_i < kvd_fArray_length; kvd_i++)
	{
		printf("\t");
		scanf("%f", kvd_pfArray + kvd_i);
	}

	// the double array
	printf("\n\n");
	printf("enter the number of elements in the double array: ");
	scanf("%u", &kvd_dArray_length);

	kvd_pdArray = (double *)malloc(sizeof(double) * kvd_dArray_length);
	if (!kvd_pdArray)
	{
		printf("\n\n");
		printf("malloc(): failed to allocate memory for %d doubles\n\n", kvd_dArray_length);
		return 1;
	}
	printf("memory allocation succeeded\n\n");

	printf("enter %d double values:\n\n", kvd_dArray_length);
	for (kvd_i = 0; kvd_i < kvd_dArray_length; kvd_i++)
	{
		printf("\t");
		scanf("%lf", kvd_pdArray + kvd_i);
	}

	// the char array
	printf("\n\n");
	printf("enter the number of elements in the char array: ");
	scanf("%u", &kvd_cArray_length);

	kvd_pcArray = (char *)malloc(sizeof(char) * kvd_cArray_length);
	if (!kvd_pcArray)
	{
		printf("\n\n");
		printf("malloc(): failed to allocate memory for %d chars\n\n", kvd_cArray_length);
		return 1;
	}
	printf("memory allocation succeeded\n\n");

	printf("enter %d chars:\n\n", kvd_cArray_length);
	getchar();  // for any previous newline character stuck in the keyboard buffer
	for (kvd_i = 0; kvd_i < kvd_cArray_length; kvd_i++)
	{
		printf("\t");
		*(kvd_pcArray + kvd_i) = getchar();
	}

	// displaying all arrays
	// the integer array
	printf("\n\n");
	printf("the integer array:\n\n");
	for (kvd_i = 0; kvd_i < kvd_iArray_length; kvd_i++)
		printf("\t%d at address = 0x%p\n", *(kvd_piArray + kvd_i), (kvd_piArray + kvd_i));

	// the float array
	printf("\n\n");
	printf("the float array:\n\n");
	for (kvd_i = 0; kvd_i < kvd_fArray_length; kvd_i++)
		printf("\t%f at address = 0x%p\n", *(kvd_pfArray + kvd_i), (kvd_pfArray + kvd_i));

	// the double array
	printf("\n\n");
	printf("the double array:\n\n");
	for (kvd_i = 0; kvd_i < kvd_dArray_length; kvd_i++)
		printf("\t%lf at address = 0x%p\n", *(kvd_pdArray + kvd_i), (kvd_pdArray + kvd_i));

	// the char array
	printf("\n\n");
	printf("the char array:\n\n");
	for (kvd_i = 0; kvd_i < kvd_cArray_length; kvd_i++)
		printf("\t'%c' at address = 0x%p\n", *(kvd_pcArray + kvd_i), (kvd_pcArray + kvd_i));

	// freeing all dynamically allocated memory in reverse order of allocation
	printf("\n\n");
	if (kvd_pcArray)
	{
		free(kvd_pcArray);
		kvd_pcArray = NULL;

		printf("freed memory block pointed to by kvd_pcArray\n");
	}
	if (kvd_pdArray)
	{
		free(kvd_pdArray);
		kvd_pdArray = NULL;

		printf("freed memory block pointed to by kvd_pdArray\n");
	}
	if (kvd_pfArray)
	{
		free(kvd_pfArray);
		kvd_pfArray = NULL;

		printf("freed memory block pointed to by kvd_pfArray\n");
	}
	if (kvd_piArray)
	{
		free(kvd_piArray);
		kvd_piArray = NULL;

		printf("freed memory block pointed to by kvd_piArray\n");
	}

	printf("\n\n");
	return 0;
}
