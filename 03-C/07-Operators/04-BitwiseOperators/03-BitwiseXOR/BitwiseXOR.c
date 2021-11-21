#include <stdio.h>

int main(void)
{
	// function prototypes
	void PrintBinaryFormOfNumber(unsigned int);

	// variable declarations
	unsigned int kvd_a, kvd_b, kvd_result;

	// code
	printf("\n\nenter kvd_a (unsigned int): ");
	scanf("%u", &kvd_a);

	printf("\n\nenter kvd_b (unsigned int): ");
	scanf("%u", &kvd_b);

	printf("\n\n");

	kvd_result = (kvd_a ^ kvd_b);
	
	PrintBinaryFormOfNumber(kvd_a);
	PrintBinaryFormOfNumber(kvd_b);
	PrintBinaryFormOfNumber(kvd_result);

	printf("Thus, (%u ^ %u) yields %u\n\n", kvd_a, kvd_b, kvd_result);

	return(0);
}

// implementation of PrintBinaryFormOfNumber()
void PrintBinaryFormOfNumber(unsigned int decimal_number)
{
	// local variable declarations
	unsigned int remainder, binary_array[8], num, i;

	// code
	for (i = 0; i < 8; i++)
		binary_array[i] = 0;

	num = decimal_number;
	i = 7;

	while (num > 0)
	{
		remainder = num % 2;
		binary_array[i--] = remainder;
		num = num / 2;
	}

	printf("binary(%u)\t= ", decimal_number);
	for (i = 0; i < 8; i++)
		printf("%u", binary_array[i]);

	printf("\n\n");
}
