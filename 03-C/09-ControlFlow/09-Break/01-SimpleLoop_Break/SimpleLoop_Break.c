#include <stdio.h>
#include <conio.h>

int main(void)
{
	// variable declarations
	int kvd_i;
	char kvd_ch;

	// code
	printf("\n\n");

	printf("printing even numbers from 1 to 100 for every user input. To exit the loop, press 'Q' or 'q'\n\n");

	for (kvd_i = 0; kvd_i <= 100; kvd_i = kvd_i + 2)
	{
		printf("\t%d\n", kvd_i);
		kvd_ch = getch();
		if (kvd_ch == 'Q' || kvd_ch == 'q')
			break;
	}

	printf("\n\n");
	printf("exiting the loop\n\n");

	return(0);
}
