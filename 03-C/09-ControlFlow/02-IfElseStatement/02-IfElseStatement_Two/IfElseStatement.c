#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_age;

	// code
	printf("\n\n");
	printf("enter age: ");
	scanf("%d", &kvd_age);
	printf("\n\n");

	if (kvd_age >= 18)
	{
		printf("entering if-block\n");
		printf("you are eligible to vote\n\n");
	}
	else
	{
		printf("entering else-block\n");
		printf("you are not eligible to vote\n\n");
	}
	printf("goodbye!\n\n");

	return(0);
}
