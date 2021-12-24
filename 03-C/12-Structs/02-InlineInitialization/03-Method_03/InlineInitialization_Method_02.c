#include <stdio.h>

int main(void)
{
	// defining a struct
	struct MyData
	{
		int i;
		float f;
		double d;
		char c;
	} kvd_data = { 5, 9.1f, 3.78623, 'N' };

	// code
	printf("\n\n");

	printf("members in kvd_data:\n\n");
	printf("\t.i = %d\n", kvd_data.i);
	printf("\t.f = %f\n", kvd_data.f);
	printf("\t.d = %lf\n", kvd_data.d);
	printf("\t.c = %c\n\n", kvd_data.c);

	return(0);
}
