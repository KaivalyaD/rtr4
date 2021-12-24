#include <stdio.h>

// defining the struct
struct MyData
{
	int i;
	float f;
	double d;
	char c;
} kvd_data = { 30, 4.5f, 11.451995, 'S' };  // inline initialization of a struct variable

int main(void)
{
	// code
	printf("\n\n");

	printf("members in kvd_data:\n\n");
	printf("\t.i = %d\n", kvd_data.i);
	printf("\t.f = %f\n", kvd_data.f);
	printf("\t.d = %lf\n", kvd_data.d);
	printf("\t.c = '%c'\n\n", kvd_data.c);

	return(0);
}
