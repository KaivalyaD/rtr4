#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_num, kvd_i_num;

	// code
	printf("\n\n");

	printf("enter an integer from where the iteration must begin: ");
	scanf("%d", &kvd_i_num);

	printf("enter the number of integers you want to print after %d: ", kvd_i_num);
	scanf("%d", &kvd_num);

	printf("\n\n");

	printf("printing integers from %d to %d\n\n", kvd_i_num, (kvd_i_num + kvd_num));
	for (kvd_i = kvd_i_num; kvd_i <= (kvd_i_num + kvd_num); kvd_i++)
		printf("\t%d\n", kvd_i);

	printf("\n\n");

	return(0);
}
