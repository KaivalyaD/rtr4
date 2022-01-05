#include <stdio.h>

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
	union DataUnion kvd_u1, kvd_u2;

	// code
	// union 1
	printf("\n\n");
	printf("members of kvd_u1:\n\n");
	
	kvd_u1.i = 27;
	kvd_u1.f = 44.25f;
	kvd_u1.d = 39.45;
	kvd_u1.c = 'S';
	
	printf("\t.i = %d\n", kvd_u1.i);
	printf("\t.f = %f\n", kvd_u1.f);
	printf("\t.d = %lf\n", kvd_u1.d);
	printf("\t.c = '%c'\n\n", kvd_u1.c);

	printf("addresses of each member in kvd_u1:\n\n");
	printf("\t&kvd_u1.i = 0x%p\n", &kvd_u1.i);
	printf("\t&kvd_u1.f = 0x%p\n", &kvd_u1.f);
	printf("\t&kvd_u1.d = 0x%p\n", &kvd_u1.d);
	printf("\t&kvd_u1.c = 0x%p\n\n", &kvd_u1.c);

	// union 2
	printf("members of kvd_u2:\n\n");

	kvd_u2.i = 52;
	printf("\t.i = %d\n", kvd_u2.i);

	kvd_u2.f = 3.14f;
	printf("\t.f = %f\n", kvd_u2.f);
	
	kvd_u2.d = 2.717829;
	printf("\t.d = %lf\n", kvd_u2.d);

	kvd_u2.c = 'P';
	printf("\t.c = '%c'\n\n", kvd_u2.c);

	printf("addresses of each member in kvd_u1:\n\n");
	printf("\t&kvd_u2.i = 0x%p\n", &kvd_u2.i);
	printf("\t&kvd_u2.f = 0x%p\n", &kvd_u2.f);
	printf("\t&kvd_u2.d = 0x%p\n", &kvd_u2.d);
	printf("\t&kvd_u2.c = 0x%p\n", &kvd_u2.c);

	printf("\n\n");
	return(0);
}
