#include <stdio.h>  // for printf() and scanf()

#define MAX(kvd_x, kvd_y)	(((kvd_x) > (kvd_y)) ? (kvd_x) : (kvd_y));

int main(int kvd_argc, char* kvd_argv[], char* kvd_envp[])
{
	// variable declarations
	int ikvd_num_1, ikvd_num_2, ikvd_result;
	float fkvd_num_1, fkvd_num_2, fkvd_result;

	// code
	printf("\n\n");

	printf("enter an integer: ");
	scanf("%d", &ikvd_num_1);
	printf("enter another integer: ");
	scanf("%d", &ikvd_num_2);

	printf("\n\n");

	ikvd_result = MAX(ikvd_num_1, ikvd_num_2);
	printf("%d is the biggest in (%d, %d)", ikvd_result, ikvd_num_1, ikvd_num_2);

	printf("\n\n");

	printf("enter a single precision floating point value: ");
	scanf("%f", &fkvd_num_1);
	printf("enter another single precision floating point value: ");
	scanf("%f", &fkvd_num_2);

	printf("\n\n");

	fkvd_result = MAX(fkvd_num_1, fkvd_num_2);
	printf("%f is the biggest in (%f, %f)", fkvd_result, fkvd_num_1, fkvd_num_2);

	printf("\n\n");
	return(0);
}
