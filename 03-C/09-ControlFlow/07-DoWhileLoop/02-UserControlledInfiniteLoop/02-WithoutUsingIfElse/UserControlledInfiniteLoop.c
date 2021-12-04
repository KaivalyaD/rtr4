#include <stdio.h>
#include <conio.h>

int main(void)
{
	// variable declarations
	char kvd_option, kvd_ch = '\0';

	// code
	printf("\n\n");

	printf("when the infinite loop begins, press 'Q' or 'q' to quit it\n\n");

	do
	{
		do
		{
			printf("inside loop\n");
			kvd_ch = getch();
		} while (kvd_ch != 'Q' && kvd_ch != 'q');

		printf("\n\n");
		printf("if you want to restart the loop, press 'Y' or 'y' and press any other key to exit\n\n");
		kvd_option = getch();
	} while (kvd_option == 'Y' || kvd_option == 'y');

	return(0);
}
