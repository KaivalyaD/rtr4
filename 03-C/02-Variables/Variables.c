#include <stdio.h>

int main(void)
{
	// variable declarations
	int i = 5;
	float f = 3.14f;
	double d = 8.041997;
	char c = 'A';

	// code
	printf("\n\n");

	printf("i = %d\n", i);
	printf("f = %f\n", f);
	printf("d = %lf\n", d);
	printf("c = %c\n", c);

	printf("\n\n");

	i = 43;
	f = 6.28f;
	d = 26.1294;
	c = 'P';

	printf("i = %d\n", i);
	printf("f = %f\n", f);
	printf("d = %lf\n", d);
	printf("c = %c\n", c);

	printf("\n\n");

	return(0);
}
