#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num_month;

	// code
	printf("\n\n");

	printf("enter month number: ");
	scanf("%d", &kvd_num_month);

	printf("\n");

	switch (kvd_num_month)
	{
	case 1:	// equivalent to an if statement
		printf("the month is January\n\n");
		break;

	case 2:	// equivalent to an else if
		printf("the month is February\n\n");
		break;

	case 3:
		printf("the month is March\n\n");
		break;

	case 4:
		printf("the month is April\n\n");
		break;

	case 5:
		printf("the month is May\n\n");
		break;

	case 6:
		printf("the month is June\n\n");
		break;

	case 7:
		printf("the month is July\n\n");
		break;

	case 8:
		printf("the month is August\n\n");
		break;

	case 9:
		printf("the month is September\n\n");
		break;

	case 10:
		printf("the month is October\n\n");
		break;

	case 11:
		printf("the month is November\n\n");
		break;

	case 12:
		printf("the month is December\n\n");
		break;

	default: // equivalent to an else
		printf("this is an invalid month\n\n");
		break;
	}

	return(0);
}
