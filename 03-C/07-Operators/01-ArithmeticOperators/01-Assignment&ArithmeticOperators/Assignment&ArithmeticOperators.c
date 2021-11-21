#include <stdio.h>

int main(void)
{
	// variable declarations
	int a, b, result;

	// code
	printf("\n\n");
	printf("enter a number: ");
	scanf("%d", &a);

	printf("\n\n");
	printf("enter another number: ");
	scanf("%d", &b);

	printf("\n\n");

	result = a + b;
	printf("sum of a=%d and b=%d is %d\n", a, b, result);

	result = a - b;
	printf("difference between a=%d and b=%d is %d\n", a, b, result);

	result = a * b;
	printf("product of a=%d and b=%d is %d\n", a, b, result);

	result = a / b;
	printf("when a=%d is divided into b=%d parts, the maximum each part gets is %d units of a\n", a, b, result);

	result = a % b;
	printf("when a=%d is divided into b=%d parts, %d units of a remain\n", a, b, result);

	printf("\n\n");
	return(0);
}
