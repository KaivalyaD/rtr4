#include <stdio.h>

int main(void)
{
	// function prototypes
	void SwapNumbers(int, int);

	// variable declarations
	int kvd_a, kvd_b;

	// code
	printf("\n\n");
	printf("enter a value for kvd_a: ");
	scanf("%d", &kvd_a);
	printf("enter a value for kvd_b: ");
	scanf("%d", &kvd_b);

	printf("\n\n");
	printf("before SwapNumbers():\n\n");
	printf("\tkvd_a = %d\n", kvd_a);
	printf("\tkvd_b = %d\n\n", kvd_b);

	SwapNumbers(kvd_a, kvd_b);

	printf("after SwapNumbers():\n\n");
	printf("\tkvd_a = %d\n", kvd_a);
	printf("\tkvd_b = %d\n\n", kvd_b);

	return(0);
}

void SwapNumbers(int kvd_x, int kvd_y)
{
	// variable declarations
	int kvd_temp;

	// code
	kvd_temp = kvd_x;
	kvd_x = kvd_y;
	kvd_y = kvd_temp;
}
