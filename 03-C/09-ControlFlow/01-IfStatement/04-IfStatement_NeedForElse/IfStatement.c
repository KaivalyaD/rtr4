#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_age;

	// code
	printf("\n\n");

	printf("enter age: ");
	scanf("%d", &kvd_age);

	if (kvd_age >= 18)
	{
		printf("you are eligible to vote\n\n");
	}
	printf("you are not eligible to vote\n\n");

	return(0);
}
