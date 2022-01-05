#include <stdio.h>

struct DataStruct
{
	int i;
	float f;
	double d;
	char c;
};

union DataUnion
{
	int i;
	float f;
	double d;
	char c;
};

int main(void)
{
	// variable declarations
	struct DataStruct kvd_struct;
	union DataUnion kvd_union;

	// code
	printf("\n\n");
	printf("members in kvd_struct:\n\n");

	kvd_struct.i = 892;
	kvd_struct.f = 71.23f;
	kvd_struct.d = 912.32121;
	kvd_struct.c = '#';

	printf("\t.i = %d\n", kvd_struct.i);
	printf("\t.f = %f\n", kvd_struct.f);
	printf("\t.d = %lf\n", kvd_struct.d);
	printf("\t.c = '%c'\n\n", kvd_struct.c);

	printf("addresses of each of the struct's members:\n\n");
	printf("\t&kvd_struct.i = 0x%p\n", &kvd_struct.i);
	printf("\t&kvd_struct.f = 0x%p\n", &kvd_struct.f);
	printf("\t&kvd_struct.d = 0x%p\n", &kvd_struct.d);
	printf("\t&kvd_struct.c = 0x%p\n\n", &kvd_struct.c);

	printf("address of kvd_struct = 0x%p\n\n", &kvd_struct);

	printf("members in kvd_union:\n\n");

	kvd_union.i = 43;
	printf("\t.i = %d\n", kvd_union.i);

	kvd_union.f = 144.127f;
	printf("\t.f = %f\n", kvd_union.f);

	kvd_union.d = 442.67162;
	printf("\t.d = %lf\n", kvd_union.d);

	kvd_union.c = 'v';
	printf("\t.c = '%c'\n\n", kvd_union.c);

	printf("addresses of each of the union's members:\n\n");
	printf("\t&kvd_union.i = 0x%p\n", &kvd_union.i);
	printf("\t&kvd_union.f = 0x%p\n", &kvd_union.f);
	printf("\t&kvd_union.d = 0x%p\n", &kvd_union.d);
	printf("\t&kvd_union.c = 0x%p\n\n", &kvd_union.c);

	printf("address of kvd_union = 0x%p", &kvd_union);

	printf("\n\n");
	return(0);
}
