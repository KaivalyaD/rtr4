#include <stdio.h>
#include <conio.h>

int main(void)
{
	// variable declarations
	char kvd_option, kvd_ch = '\0';

	// code
	printf("\n\n");

	printf("when the infinite loop begins, enter 'Q' or 'q' to quit it\n\n");

	printf("enter 'Y' or 'y' to initiate the infinite loop: ");
	kvd_option = getch();
	printf("\n\n");

	if (kvd_option == 'Y' || kvd_option == 'y')
	{
		while (1)
		{
			printf("inside loop\n");
			kvd_ch = getch();
			if (kvd_ch == 'Q' || kvd_ch == 'q')
				break;
		}

		printf("\n\n");
		printf("exiting the user-controlled infinite loop\n\n");
	}
	else
		printf("invalid option entered\n\n");

	return(0);
}
