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

	printf("\n\n");

	printf("choose an option:\n");
	printf("\t'A' or 'a' for addition\n");
	printf("\t'S' or 's' for absolute subtraction\n");
	printf("\t'M' or 'm' for multiplication\n");
	printf("\t'D' or 'd' for division\n\n");

	printf("your choice: ");
	kvd_option = getch();

	printf("\n\n");

	if (kvd_option == 'A' || kvd_option == 'a')
	{
		kvd_result = kvd_a + kvd_b;
		printf("addition of %d and kvd_b %d is %d\n\n", kvd_a, kvd_b, kvd_result);
	}
	
	else if (kvd_option == 'S' || kvd_option == 's')
	{
		if (kvd_a > kvd_b)
			kvd_result = kvd_a - kvd_b;
		else
			kvd_result = kvd_b - kvd_a;
		printf("absolute value of the result of subtracting of %d from %d is %d\n\n", kvd_a, kvd_b, kvd_result);
	}

	else if (kvd_option == 'M' || kvd_option == 'm')
	{
		kvd_result = kvd_a * kvd_b;
		printf("multiplication of %d and %d is %d\n\n", kvd_a, kvd_b, kvd_result);
	}

	else if (kvd_option == 'D' || kvd_option == 'd')
	{
		printf("choose an option:\n");
		printf("\t'Q' or 'q' for the quotient on division\n");
		printf("\t'R' or 'r' for the remainder on division\n\n");

		printf("your choice: ");
		kvd_option_division = getch();

		printf("\n\n");

		if (kvd_option_division == 'Q' || kvd_option_division == 'q')
		{
			kvd_result = kvd_a / kvd_b;
			printf("division of %d by %d yields %d\n\n", kvd_a, kvd_b, kvd_result);
		}
		
		else if (kvd_option_division == 'R' || kvd_option_division == 'r')
		{
			kvd_result = kvd_a % kvd_b;
			printf("when %d is divided into %d parts, %d parts of %d remain\n\n", kvd_a, kvd_b, kvd_result, kvd_a);
		}

		else
		{
			printf("%c is an invalid option\n\n", kvd_option_division);
		}
	}

	else
	{
		printf("that is an invalid option\n\n", kvd_option);
	}

	printf("the if-else if-else ladder is over\n\n");
	return(0);
}
