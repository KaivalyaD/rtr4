#include <stdio.h>
#include <stdlib.h>

typedef struct Data *DataPtr;
struct Data
{
	int i;
	float f;
	double d;
};

int main(void)
{
	// variable declarations
	DataPtr kvd_pData = NULL;
	int kvd_i_size, kvd_f_size, kvd_d_size, kvd_struct_size, kvd_pData_size;
	
	// code
	printf("\n\n");

	kvd_pData = (DataPtr)malloc(sizeof(struct Data) * 1);
	if (!kvd_pData)
	{
		printf("malloc(): failed to allocate memory for 1 struct Data\n\n");
		return 1;
	}
	printf("memory allocated successfully\n\n");

	kvd_pData->i = 90;
	kvd_pData->f = 30.2f;
	kvd_pData->d = 1.9823;

	kvd_i_size = sizeof(kvd_pData->i);
	kvd_f_size = sizeof(kvd_pData->f);
	kvd_d_size = sizeof(kvd_pData->d);
	kvd_struct_size = sizeof(struct Data);
	kvd_pData_size = sizeof(DataPtr);

	printf("members in *kvd_pData:\n\n");
	printf("\t.i = %d\n", kvd_pData->i);
	printf("\t.f = %f\n", kvd_pData->f);
	printf("\t.d = %lf\n\n", kvd_pData->d);

	printf("sizes of each member in *kvd_pData:\n\n");
	printf("\tsizeof(kvd_pData->i) = %d\n", kvd_i_size);
	printf("\tsizeof(kvd_pData->f) = %d\n", kvd_f_size);
	printf("\tsizeof(kvd_pData->d) = %d\n\n", kvd_d_size);

	printf("sizeof(struct Data) = %d\n", kvd_struct_size);
	printf("sizeof(DataPtr) = %d\n\n", kvd_pData_size);

	if (kvd_pData)
	{
		free(kvd_pData);
		kvd_pData = NULL;

		printf("freed and cleaned all dynamically allocated memory\n\n");
	}

	return 0;
}
