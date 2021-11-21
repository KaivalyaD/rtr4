#include <stdio.h>

int main(void)
{
	// function prototypes
	void PrintBinaryFormOfNumber(unsigned int);

	// variable declarations
	unsigned int kvd_a, kvd_b, kvd_result;

	// code
	printf("\n\nenter kvd_a (int): ");
	scanf("%d", &kvd_a);

	printf("\n\nenter kvd_b (int): ");
	scanf("%d", &kvd_b);

	printf("\n\n");
	
	kvd_result = (kvd_a & kvd_b);

	PrintBinaryFormOfNumber(kvd_a);
	PrintBinaryFormOfNumber(kvd_b);
	PrintBinaryFormOfNumber(kvd_result);

	printf("Thus, (%d & %d) yields %d\n\n", kvd_a, kvd_b, kvd_result);
}

// purpose: to print the 8-bit binary form of the argument to stdout
void PrintBinaryFormOfNumber(unsigned int decimal_number)
{
	// variable declarations
	unsigned int quotient, remainder;
	unsigned int num;
	unsigned int binary_array[8];
	int i;

	// code
	for (i = 0; i < 8; i++)
		binary_array[i] = 0;

	num = decimal_number;
	i = 7;
	while (num != 0)
	{
		quotient = num / 2;
		remainder = num % 2;
		binary_array[i] = remainder;
		num = quotient;
		i--;
	}

	printf("binary(%u)\t= ", decimal_number);
	for (i = 0; i < 8; i++)
		printf("%u", binary_array[i]);

	printf("\n\n");
}
