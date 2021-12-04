#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_num_month;

	// code
	printf("\n\n");

	printf("enter the month number: ");
	scanf("%d", &kvd_num_month);

	printf("\n");

	// if-else if-else ladder begins
	if (kvd_num_month == 1)
		printf("the month is January\n\n");

	else if (kvd_num_month == 2)
		printf("the month is February\n\n");

	else if (kvd_num_month == 3)
		printf("the month is March\n\n");

	else if (kvd_num_month == 4)
		printf("the month is April\n\n");

	else if (kvd_num_month == 5)
		printf("the month is May\n\n");

	else if (kvd_num_month == 6)
		printf("the month is June\n\n");

	else if (kvd_num_month == 7)
		printf("the month is July\n\n");

	else if (kvd_num_month == 8)
		printf("the month is August\n\n");

	else if (kvd_num_month == 9)
		printf("the month is September\n\n");

	else if (kvd_num_month == 10)
		printf("the month is October\n\n");

	else if (kvd_num_month == 11)
		printf("the month is November\n\n");

	else if (kvd_num_month == 12)
		printf("the month is December\n\n");

	else
		printf("that is an invalid month number\n\n");

	return(0);
}
