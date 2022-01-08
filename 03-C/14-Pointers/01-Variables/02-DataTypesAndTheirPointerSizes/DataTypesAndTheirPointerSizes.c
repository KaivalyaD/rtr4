#include <stdio.h>

struct Employee
{
	char name[100];
	int age;
	float salary;
	char sex;
	char marital_status;
};

int main(void)
{
	// code
	printf("\n\n");

	printf("sizeof(int)\t= %zd\tsizeof(int *) = %zd\n\n", sizeof(int), sizeof(int *));
	printf("sizeof(float)\t= %zd\tsizeof(float *) = %zd\n\n", sizeof(float), sizeof(float *));
	printf("sizeof(double)\t= %zd\tsizeof(double *) = %zd\n\n", sizeof(double), sizeof(double *));
	printf("sizeof(char)\t= %zd\tsizeof(char *) = %zd\n\n", sizeof(char), sizeof(char *));
	printf("sizeof(struct Employee)\t= %zd\tsizeof(struct Employee *) = %zd\n\n", sizeof(struct Employee), sizeof(struct Employee *));

	return 0;
}
