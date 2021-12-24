#include <stdio.h>

// defining a struct
struct MyData
{
	int i;
	float f;
	double d;
	char c;
} kvd_data;  // declaring a single global variable of type struct MyData

int main(void)
{
	// variable declarations
	int kvd_size_int, kvd_size_float, kvd_size_double, kvd_size_char, kvd_size_struct;
	
	// code
	// assigning values to each members of kvd_data
	kvd_data.i = 30;
	kvd_data.f = 11.45f;
	kvd_data.d = 1.2995;
	kvd_data.c = 'K';

	// displaying them
	printf("\n\n");
	printf("members of kvd_data hold:\n\n");
	printf("\t.i = %d\n", kvd_data.i);
	printf("\t.f = %f\n", kvd_data.f);
	printf("\t.d = %lf\n", kvd_data.d);
	printf("\t.c = '%c'\n", kvd_data.c);

	// calculating their sizes (in bytes)
	kvd_size_int = sizeof(kvd_data.i);
	kvd_size_float = sizeof(kvd_data.f);
	kvd_size_double = sizeof(kvd_data.d);
	kvd_size_char = sizeof(kvd_data.c);

	// displaying the sizes
	printf("\n\n");
	printf("sizes of each member in kvd_data:\n\n");
	printf("\ti = %d\n", kvd_size_int);
	printf("\tf = %d\n", kvd_size_float);
	printf("\td = %d\n", kvd_size_double);
	printf("\tc = %d\n", kvd_size_char);

	// calculating the size in bytes of the entire struct
	kvd_size_struct = sizeof(struct MyData);
	printf("\n\n");
	printf("size of struct MyData = %d (!= sum of all members' sizes necessarily)\n\n", kvd_size_struct);

	return(0);
}
