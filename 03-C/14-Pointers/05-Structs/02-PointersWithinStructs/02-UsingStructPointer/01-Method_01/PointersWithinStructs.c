#include <stdio.h>
#include <stdlib.h>

struct Data
{
	int *ptr_i;
	int i;

	float *ptr_f;
	float f;

	double *ptr_d;
	double d;
};

int main(void)
{
	// variable declarations
	struct Data *kvd_pData = NULL;

	// code
	printf("\n\n");

	kvd_pData = (struct Data *)malloc(sizeof(struct Data) * 1);
	if (!kvd_pData)
	{
		printf("malloc(): failed to allocate memory for 1 struct Data\n\n");
		return 1;
	}
	printf("memory allocated successfully\n\n");
	
	(*kvd_pData).i = 45;
	(*kvd_pData).ptr_i = &(*kvd_pData).i;

	(*kvd_pData).f = 2.71f;
	(*kvd_pData).ptr_f = &(*kvd_pData).f;

	(*kvd_pData).d = 3.14159;
	(*kvd_pData).ptr_d = &(*kvd_pData).d;

	printf("*((*kvd_pData).ptr_i) = %d\n", *((*kvd_pData).ptr_i));
	printf("address = 0x%p\n\n", (*kvd_pData).ptr_i);

	printf("*((*kvd_pData).ptr_f) = %f\n", *((*kvd_pData).ptr_f));
	printf("address = 0x%p\n\n", (*kvd_pData).ptr_f);

	printf("*((*kvd_pData).ptr_d) = %lf\n", *((*kvd_pData).ptr_d));
	printf("address = 0x%p\n\n", (*kvd_pData).ptr_d);

	if (kvd_pData)
	{
		free(kvd_pData);
		kvd_pData = NULL;

		printf("freed and cleaned all dynamically allocated memory\n\n");
	}

	return 0;
}
