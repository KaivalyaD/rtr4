#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_a = 5, kvd_b = 10;

	// code
	printf("\n\n");
	printf("kvd_a \t= %d\n", kvd_a);
	printf("kvd_a++\t= %d\n", kvd_a++);
	printf("kvd_a\t= %d\n", kvd_a);
	printf("++kvd_a\t= %d\n\n", ++kvd_a);

	printf("kvd_b\t= %d\n", kvd_b);
	printf("kvd_b--\t= %d\n", kvd_b--);
	printf("kvd_b\t= %d\n", kvd_b);
	printf("--kvd_b\t= %d\n\n", --kvd_b);

	return(0);
}
