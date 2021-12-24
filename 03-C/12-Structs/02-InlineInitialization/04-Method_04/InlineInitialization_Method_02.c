#include <stdio.h>

// defining the struct
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
	struct MyData kvd_data_one = { 35, 3.9f, 1.23765, 'K' };
	struct MyData kvd_data_two = { 'V', 6.2f, 12.199523, 68 };
	struct MyData kvd_data_three = { 36, 'D' };
	struct MyData kvd_data_four = { 79 };

	// code
	printf("\n\n");

	printf("members of struct MyData kvd_data_one:\n\n");
	printf("\t.i = %d\n", kvd_data_one.i);
	printf("\t.f = %f\n", kvd_data_one.f);
	printf("\t.d = %lf\n", kvd_data_one.d);
	printf("\t.c = '%c'\n\n", kvd_data_one.c);

	printf("members of struct MyData kvd_data_two:\n\n");
	printf("\t.i = %d\n", kvd_data_two.i);
	printf("\t.f = %f\n", kvd_data_two.f);
	printf("\t.d = %lf\n", kvd_data_two.d);
	printf("\t.c = '%c'\n\n", kvd_data_two.c);

	printf("members of struct MyData kvd_data_three:\n\n");
	printf("\t.i = %d\n", kvd_data_three.i);
	printf("\t.f = %f\n", kvd_data_three.f);
	printf("\t.d = %lf\n", kvd_data_three.d);
	printf("\t.c = '%c'\n\n", kvd_data_three.c);

	printf("members of struct MyData kvd_data_four:\n\n");
	printf("\t.i = %d\n", kvd_data_four.i);
	printf("\t.f = %f\n", kvd_data_four.f);
	printf("\t.d = %lf\n", kvd_data_four.d);
	printf("\t.c = '%c'\n\n", kvd_data_four.c);

	return(0);
}
