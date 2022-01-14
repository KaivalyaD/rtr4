#include <stdio.h>
#include <stdlib.h>

int main(void)
{
	// function prototypes
	int MyIntegerFactory(int **, unsigned int);

	// variable declarations
	int *kvd_piArray = NULL, kvd_i, kvd_ret;
	unsigned int kvd_element_count;

	// code
	printf("\n\n");
	printf("enter the number of elements you want in your array: ");
	scanf("%d", &kvd_element_count);

	kvd_ret = MyIntegerFactory(&kvd_piArray, kvd_element_count);
	if (kvd_ret)
	{
		printf("MyIntegerFactory(): failed to allocate memory for %u integers\n\n", kvd_element_count);
		return 1;
	}
	printf("\n\nsuccessfully allocated memory for %u integers\n\n", kvd_element_count);
	
	printf("enter %u elements:\n\n", kvd_element_count);
	for (kvd_i = 0; kvd_i < kvd_element_count; kvd_i++)
	{
		printf("\t");
		scanf("%d", kvd_piArray + kvd_i);
	}

	printf("\n\n");
	printf("you entered:\n\n");
	for (kvd_i = 0; kvd_i < kvd_element_count; kvd_i++)
		printf("\t%d\n", *(kvd_piArray + kvd_i));
	
	printf("\n\n");
	if (kvd_piArray)
	{
		free(kvd_piArray);
		kvd_piArray = NULL;

		printf("freed and cleaned memory block pointed to by kvd_piArray\n\n");
	}

	return 0;
}

int MyIntegerFactory(int **kvd_ppiBeginning, unsigned int kvd_count)
{
	// code
	*kvd_ppiBeginning = NULL;

	*kvd_ppiBeginning = (int *)malloc(sizeof(int) * kvd_count);
	if (!(*kvd_ppiBeginning))
		return 1;

	return 0;
}
