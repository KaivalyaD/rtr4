#include <stdio.h>

int main(void)
{
	// variable declarations
	char kvd_cArray_1[] = { 'A', 'S', 'T', 'R', 'O', 'M', 'E', 'D', 'I', 'C', 'O', 'M', 'P' , '\0'};
	char kvd_cArray_2[9] = { 'W', 'E', 'L', 'C', 'O', 'M', 'E', 'S', '\0'};
	char kvd_cArray_3[] = { 'Y', 'O', 'U', '\0' };
	char kvd_cArray_4[3] = "TO\0";
	char kvd_cArray_5[] = "REAL TIME RENDERING BATCH OF 2021-22";
	
	char kvd_cArray_WithoutNullTerminator[] = { 'H', 'E', 'L', 'L', 'O' };

	// code
	printf("\n\n");

	printf("size of kvd_cArray_1[] = %zu\n", sizeof(kvd_cArray_1));
	printf("size of kvd_cArray_2[] = %zu\n", sizeof(kvd_cArray_2));
	printf("size of kvd_cArray_3[] = %zu\n", sizeof(kvd_cArray_3));
	printf("size of kvd_cArray_4[] = %zu\n", sizeof(kvd_cArray_4));
	printf("size of kvd_cArray_5[] = %zu\n", sizeof(kvd_cArray_5));

	printf("\n\n");

	printf("the strings are:\n\n");
	printf("\tkvd_cArray_1[]: \"%s\"\n", kvd_cArray_1);
	printf("\tkvd_cArray_2[]: \"%s\"\n", kvd_cArray_2);
	printf("\tkvd_cArray_3[]: \"%s\"\n", kvd_cArray_3);
	printf("\tkvd_cArray_4[]: \"%s\"\n", kvd_cArray_4);
	printf("\tkvd_cArray_5[]: \"%s\"\n", kvd_cArray_5);

	printf("\n\n");

	printf("size of kvd_cArray_WithoutNullTerminator: %zu\n", sizeof(kvd_cArray_WithoutNullTerminator));
	// the following may or may not display garbage characters
	printf("\n\tkvd_cArray_WithoutNullTerminator: \"%s\"\n", kvd_cArray_WithoutNullTerminator);

	return(0);
}
