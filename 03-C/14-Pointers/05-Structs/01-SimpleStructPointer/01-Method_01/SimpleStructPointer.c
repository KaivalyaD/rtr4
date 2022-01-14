#include <stdio.h>
#include <stdlib.h>

struct Data
{
	int i;
	float f;
	double d;
	char c;
};

int main(void)
{
	// variable declarations
	struct Data *kvd_pData = NULL;
	int kvd_i_size, kvd_f_size, kvd_d_size, kvd_c_size, kvd_struct_size, kvd_pData_size;

	// code
	printf("\n\n");
	
	// allocation
	kvd_pData = (struct Data *)malloc(sizeof(kvd_pData) * 1);
	if (!kvd_pData)
	{
		printf("malloc(): failed to allocate memory for 1 struct kvd_pData\n\n");
		return 1;
	}
	printf("successfully allocated memory\n\n");

	// initialization
	(*kvd_pData).i = 13;
	(*kvd_pData).f = 1.232f;
	(*kvd_pData).d = 1.2221;
	(*kvd_pData).c = 'N';

	// display
	printf("members in struct Data *kvd_pData:\n\n");
	printf("\t.i = %d\n", (*kvd_pData).i);
	printf("\t.f = %f\n", (*kvd_pData).f);
	printf("\t.d = %lf\n", (*kvd_pData).d);
	printf("\t.c = '%c'\n\n", (*kvd_pData).c);

	// sizes of each member
	kvd_i_size = sizeof((*kvd_pData).i);
	kvd_f_size = sizeof((*kvd_pData).f);
	kvd_d_size = sizeof((*kvd_pData).d);
	kvd_c_size = sizeof((*kvd_pData).c);
	kvd_struct_size = sizeof(*kvd_pData);
	kvd_pData_size = sizeof(kvd_pData);

	printf("sizes in bytes of each member:\n\n");
	printf("\tsizeof((*kvd_pData).i) = %d\n", kvd_i_size);
	printf("\tsizeof((*kvd_pData).f) = %d\n", kvd_f_size);
	printf("\tsizeof((*kvd_pData).d) = %d\n", kvd_d_size);
	printf("\tsizeof((*kvd_pData).c) = %d\n\n", kvd_c_size);

	// the size of the entire struct in bytes
	printf("sizeof(struct Data) = %d\n", kvd_struct_size);
	printf("sizeof(struct Data *) = %d\n\n", kvd_pData_size);

	if (kvd_pData)
	{
		free(kvd_pData);
		kvd_pData = NULL;

		printf("freed and cleaned all dynamically allocated memory\n\n");
	}

	return 0;
}
