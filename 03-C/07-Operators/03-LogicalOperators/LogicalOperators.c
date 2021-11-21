#include <stdio.h>

int main(void)
{
	// variable declarations
	int kvd_a, kvd_b, kvd_c, kvd_result;

	// code
	printf("\n\n");
	printf("enter kvd_a (int): ");
	scanf("%d", &kvd_a);

	printf("\n\nenter kvd_b (int): ");
	scanf("%d", &kvd_b);

	printf("\n\nenter kvd_c (int): ");
	scanf("%d", &kvd_c);

	printf("\n\n");
	printf("when you read a 0, the answer is 'FALSE'; and\nwhen you read a 1, the answer is 'TRUE'\n\n");

	kvd_result = (kvd_a <= kvd_b) && (kvd_b != kvd_c);
	printf("(%d <= %d) && (%d != %d) yields\t%d\n", kvd_a, kvd_b, kvd_b, kvd_c, kvd_result);

	kvd_result = (kvd_b >= kvd_a) || (kvd_a == kvd_c);
	printf("(%d >= %d) || (%d == %d) yields\t%d\n\n", kvd_b, kvd_a, kvd_a, kvd_c, kvd_result);

	kvd_result = !kvd_a;
	printf("!%d yields\t%d\n", kvd_a, kvd_result);

	kvd_result = !kvd_b;
	printf("!%d yields\t%d\n", kvd_b, kvd_result);

	kvd_result = !kvd_c;
	printf("!%d yields\t%d\n\n", kvd_c, kvd_result);

	kvd_result = (!(kvd_a <= kvd_b) && !(kvd_b != kvd_c));
	printf("(!(%d <= %d) && !(%d != %d)) yields\t%d\n", kvd_a, kvd_b, kvd_b, kvd_c, kvd_result);

	kvd_result = !((kvd_b >= kvd_a) || (kvd_a == kvd_c));
	printf("!((%d >= %d) || (%d == %d)) yields\t%d\n\n", kvd_b, kvd_a, kvd_a, kvd_c, kvd_result);

	return(0);
}
