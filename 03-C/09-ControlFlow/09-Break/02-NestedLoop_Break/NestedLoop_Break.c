#include <stdio.h>
#include <conio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_j;

	// code
	printf("\n\n");

	for (kvd_i = 1; kvd_i <= 20; kvd_i++)
	{
		for (kvd_j = 1; kvd_j <= 20; kvd_j++)
		{
			if (kvd_j > kvd_i)
				break;
			printf("* ");
		}

		printf("\n");
	}

	printf("\n\n");

	return(0);
}
