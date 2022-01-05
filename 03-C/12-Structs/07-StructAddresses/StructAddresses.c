#include <stdio.h>

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
	struct Data kvd_data;

	// code
	kvd_data.i = 75;
	kvd_data.f = 892.1902;
	kvd_data.d = 3.14159;
	kvd_data.c = 'N';

	printf("\n\n");
	printf("members in kvd_data:\n\n");
	printf("\t.i = %d\n", kvd_data.i);
	printf("\t.f = %f\n", kvd_data.f);
	printf("\t.d = %lf\n", kvd_data.d);
	printf("\t.c = '%c'\n\n", kvd_data.c);

	printf("addresses of all members in kvd_data:\n\n");
	printf("\t&kvd_data.i = 0x%p\n", &kvd_data.i);
	printf("\t&kvd_data.f = 0x%p\n", &kvd_data.f);
	printf("\t&kvd_data.d = 0x%p\n", &kvd_data.d);
	printf("\t&kvd_data.c = 0x%p\n", &kvd_data.c);

	printf("\n\n");

	printf("starting address of kvd_data = %p\n\n", &kvd_data);

	return(0);
}
