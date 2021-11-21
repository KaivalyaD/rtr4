#include <stdio.h>

int main(void)
{
	// code
	printf("\n\n");

	printf("Size of 'int'\t\t= %zd bytes\n", sizeof(int));
	printf("Size of 'unsigned int'\t= %zd bytes\n", sizeof(unsigned int));
	printf("Size of 'long'\t\t= %zd bytes\n", sizeof(long));
	printf("Size of 'long long'\t= %zd bytes\n", sizeof(long long));

	printf("Size of 'float'\t\t= %zd bytes\n", sizeof(float));
	printf("Size of 'double'\t= %zd bytes\n", sizeof(double));
	printf("Size of 'long double'\t= %zd bytes\n", sizeof(long double));

	printf("\n\n");

	return(0);
}
