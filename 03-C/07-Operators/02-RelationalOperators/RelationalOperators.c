#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_a, kvd_b, kvd_result;

	// code
	printf("\n\n");
	printf("enter kvd_a (int): ");
	scanf("%d", &kvd_a);

	printf("\n\nenter kvd_b (int): ");
	scanf("%d", &kvd_b);

	printf("\n\nwhen you read a 0, the answer if 'FALSE'; and\nwhen you read a 1, the answer is 'TRUE'\n\n");

	kvd_result = (kvd_a < kvd_b);
	printf("(kvd_a < kvd_b) yields a\t%d\n", kvd_result);

	kvd_result = (kvd_a > kvd_b);
	printf("(kvd_a > kvd_b) yields a\t%d\n", kvd_result);

	kvd_result = (kvd_a <= kvd_b);
	printf("(kvd_a <= kvd_b) yields a\t%d\n", kvd_result);

	kvd_result = (kvd_a >= kvd_b);
	printf("(kvd_a >= kvd_b) yields a\t%d\n", kvd_result);

	kvd_result = (kvd_a == kvd_b);
	printf("(kvd_a == kvd_b) yields a\t%d\n", kvd_result);

	kvd_result = (kvd_a != kvd_b);
	printf("(kvd_a != kvd_b) yields a\t%d\n", kvd_result);

	return(0);
}
