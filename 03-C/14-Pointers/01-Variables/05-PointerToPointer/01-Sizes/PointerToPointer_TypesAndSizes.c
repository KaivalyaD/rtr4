#include <stdio.h>

struct Employee
{
	char name[100], sex, marital_status;
	int age;
	float salary;
};

int main(void)
{
	// code
	printf("\n\n");

	printf("sizeof(int) = %zd; sizeof(int *) = %zd; sizeof(int **) = %zd\n\n", sizeof(int), sizeof(int *), sizeof(int **));
	printf("sizeof(float) = %zd; sizeof(float *) = %zd; sizeof(float **) = %zd\n\n", sizeof(float), sizeof(float *), sizeof(float **));
	printf("sizeof(double) = %zd; sizeof(double *) = %zd; sizeof(double **) = %zd\n\n", sizeof(double), sizeof(double *), sizeof(double **));
	printf("sizeof(char) = %zd; sizeof(char *) = %zd; sizeof(char **) = %zd\n\n", sizeof(char), sizeof(char *), sizeof(char **));
	printf("sizeof(struct Employee) = %zd; sizeof(struct Employee *) = %zd; sizeof(struct Employee **) = %zd\n\n", sizeof(struct Employee), sizeof(struct Employee *), sizeof(struct Employee **));

	return(0);
}
