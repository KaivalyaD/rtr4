#include <stdio.h> // for printf()
#include <conio.h> // for getch()

int main(void)
{
	// variable declarations
	int kvd_a, kvd_b, kvd_result;
	char kvd_option, kvd_option_division;

	// code
	printf("\n\n");

	printf("enter a value for kvd_a: ");
	scanf("%d", &kvd_a);

	printf("enter a value for kvd_b: ");
	scanf("%d", &kvd_b);

	printf("choose an option:\n");
	printf("\t'A' or 'a' for addition\n");
	printf("\t'S' or 's' for absolute subtraction\n");
	printf("\t'M' or 'm' for multiplication\n");
	printf("\t'D' or 'd' for division\n\n");

	printf("your choice: ");
	kvd_option = getch();

	printf("\n\n");

	switch (kvd_option)
	{
	case 'A':
	case 'a':
		kvd_result = kvd_a + kvd_b;
		printf("addition of %d and %d is %d\n\n", kvd_a, kvd_b, kvd_result);
		break;

	case 'S':
	case 's':
		if (kvd_a > kvd_b)
		{
			kvd_result = kvd_a - kvd_b;
			printf("subtraction of %d and %d is %d\n\n", kvd_a, kvd_b, kvd_result);
		}
		else
		{
			kvd_result = kvd_b - kvd_a;
			printf("subtraction of %d and %d is %d\n\n", kvd_a, kvd_b, kvd_result);
		}
		break;

	case 'M':
	case 'm':
		kvd_result = kvd_a * kvd_b;
		printf("multiplication of %d and %d is %d\n\n", kvd_a, kvd_b, kvd_result);
		break;

	case 'D':
	case 'd':
		printf("choose an option:\n");
		printf("'Q', 'q', or '/' for quotient upon division\n");
		printf("'R', 'r', or '%%' for remainder upon division\n\n");

		printf("your choice: ");
		kvd_option_division = getch();

		printf("\n\n");

		switch (kvd_option_division)
		{
		case 'Q':
		case 'q':
		case '/':
			kvd_result = kvd_a / kvd_b;
			printf("division of %d and %d yields %d as the quotient\n\n", kvd_a, kvd_b, kvd_result);
			break;

		case 'R':
		case 'r':
		case '%':
			kvd_result = kvd_a % kvd_b;
			printf("when %d is divided into %d parts, %d parts of %d remain undistributed\n\n", kvd_a, kvd_b, kvd_result, kvd_a);
			break;

		default:
			printf("'%c' is an invalid option\n\n", kvd_option_division);
			break;
		}
		break;

	default:
		printf("'%c' is an invalid option\n\n", kvd_option);
		break;
	}

	printf("switch case statements covered\n\n");

	return(0);
}
