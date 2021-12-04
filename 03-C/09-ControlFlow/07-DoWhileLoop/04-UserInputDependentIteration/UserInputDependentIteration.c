#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_num, kvd_i_num;

	// code
	printf("\n\n");

	printf("enter an integer from where to begin the iteration: ");
	scanf("%d", &kvd_i_num);

	printf("enter the number of numbers to print after %d: ", kvd_i_num);
	scanf("%d", &kvd_num);

	printf("\n\n");
	
	printf("printing numbers from %d to %d\n\n", kvd_i_num, (kvd_i_num + kvd_num));

	kvd_i = kvd_i_num;
	do
	{
		printf("\t%d\n", kvd_i);
		kvd_i++;
	} while (kvd_i <= (kvd_i_num + kvd_num));

	printf("\n\n");

	return(0);
}
