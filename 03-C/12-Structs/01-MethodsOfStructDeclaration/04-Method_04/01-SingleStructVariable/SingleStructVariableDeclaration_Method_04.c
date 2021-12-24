#include <stdio.h>

// defining the struct globally
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
	int kvd_i_size, kvd_f_size, kvd_d_size, kvd_c_size, kvd_MyData_size;

	// code
	kvd_data.i = 30;
	kvd_data.f = 11.45f;
	kvd_data.d = 1.2995;
	kvd_data.c = 'K';

	printf("\n\n");
	
	printf("values of all members in kvd_data:\n\n");
	printf("\t.i = %d\n", kvd_data.i);
	printf("\t.f = %f\n", kvd_data.f);
	printf("\t.d = %lf\n", kvd_data.d);
	printf("\t.c = %c\n\n", kvd_data.c);

	kvd_i_size = sizeof(kvd_data.i);
	kvd_f_size = sizeof(kvd_data.f);
	kvd_d_size = sizeof(kvd_data.d);
	kvd_c_size = sizeof(kvd_data.c);
	kvd_MyData_size = sizeof(struct MyData);

	printf("size in bytes of each member in kvd_data:\n\n");
	printf("\ti = %d\n", kvd_i_size);
	printf("\tf = %d\n", kvd_f_size);
	printf("\td = %d\n", kvd_d_size);
	printf("\tc = %d\n\n", kvd_c_size);

	printf("size of the entire struct MyData = %d\n\n", kvd_MyData_size);

	return(0);
}
