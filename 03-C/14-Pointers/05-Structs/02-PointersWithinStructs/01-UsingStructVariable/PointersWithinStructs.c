#include <stdio.h>

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
	struct Data kvd_data;

	// code
	kvd_data.i = 2;
	kvd_data.ptr_i = &kvd_data.i;

	kvd_data.f = 3.14f;
	kvd_data.ptr_f = &kvd_data.f;

	kvd_data.d = 82.39293;
	kvd_data.ptr_d = &kvd_data.d;

	printf("\n\n");

	printf("kvd_data.i = %d\n", *(kvd_data.ptr_i));
	printf("address of kvd_data.i = 0x%p\n\n", kvd_data.ptr_i);

	printf("kvd_data.f = %f\n", *(kvd_data.ptr_f));
	printf("address of kvd_data.f = 0x%p\n\n", kvd_data.ptr_f);

	printf("kvd_data.d = %lf\n", *(kvd_data.ptr_d));
	printf("address of kvd_data.d = 0x%p\n\n", kvd_data.ptr_d);

	return 0;
}
