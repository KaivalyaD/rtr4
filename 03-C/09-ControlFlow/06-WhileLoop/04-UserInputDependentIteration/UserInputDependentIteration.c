#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_i, kvd_num, kvd_i_num;

	// code
	printf("\n\n");

	printf("enter an integer from where the iteration begins: ");
	scanf("%d", &kvd_i_num);

	kvd_i = kvd_i_num;

	printf("enter the number of numbers to be printed after %d: ", kvd_i);
	scanf("%d", &kvd_num);

	printf("\n\nprinting numbers from %d to %d:\n\n", kvd_i, (kvd_i_num + kvd_num));

	while (kvd_i <= (kvd_i_num + kvd_num))
	{
		printf("\t%d\n", kvd_i);
		kvd_i++;
	}

	printf("\n\n");

	return(0);
}
