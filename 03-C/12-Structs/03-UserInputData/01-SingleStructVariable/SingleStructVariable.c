#include <stdio.h>
#include <conio.h>

struct MyData
{
	int i;
	float f;
	double d;
	char c;
};

int main(void)
{
	// variable declarations
	struct MyData kvd_data;

	// code
	printf("\n\n");

	printf("enter (int) kvd_data.i: ");
	scanf("%d", &kvd_data.i);

	printf("enter (float) kvd_data.f: ");
	scanf("%f", &kvd_data.f);

	printf("enter (double) kvd_data.d: ");
	scanf("%lf", &kvd_data.d);

	printf("enter (char) kvd_data.c: ");
	kvd_data.c = getch();
	printf("%c", kvd_data.c);

	printf("\n\n");
	printf("you entered:\n\n");
	printf("\tkvd_data.i = %d\n", kvd_data.i);
	printf("\tkvd_data.f = %f\n", kvd_data.f);
	printf("\tkvd_data.d = %lf\n", kvd_data.d);
	printf("\tkvd_data.c = '%c'\n\n", kvd_data.c);

	return(0);
}
