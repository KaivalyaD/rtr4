#include <stdio.h>
#include <conio.h>

int main(void)
{
	// variable declarations
	char kvd_option, kvd_ch = '\0';

	// code
	printf("\n\n");

	printf("once the infinite loop begins, enter 'Q' or 'q' to quit it\n\n");

	printf("enter 'Y' or 'y' to initiate the infinite loop: ");
	kvd_option = getch();
	printf("\n\n");
	if (kvd_option == 'Y' || kvd_option == 'y')
	{
		for (;;)  // entering the infinite loop
		{
			printf("looping...\n");
			kvd_ch = getch();
			if (kvd_ch == 'Q' || kvd_ch == 'q')
				break;  // exiting the infinite loop
		}
	}
	else
	{
		printf("%c is not an appropriate choice\n\n", kvd_option);
		return(0);
	}

	printf("\n\n");
	printf("exiting user-controlled infinite loop\n\n");

	return(0);
}
