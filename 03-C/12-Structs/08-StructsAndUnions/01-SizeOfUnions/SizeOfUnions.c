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
	printf("size of struct DataStruct = %zu\n", sizeof(kvd_struct));
	printf("\n\n");
	printf("size of union DataUnion = %zu\n", sizeof(kvd_union));
	printf("\n\n");

	return(0);
}
