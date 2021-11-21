#include <stdio.h>

int main(void)
{
	// function prototypes
	void PrintBinaryFormOfNumber(unsigned int);

	// variable declarations
	unsigned int kvd_a, kvd_num_bits, kvd_result;

	// code
	printf("\n\nenter kvd_a (unsigned int): ");
	scanf("%u", &kvd_a);

	printf("\n\nnumber of bits by which to shift kvd_a=%u to the right: ", kvd_a);
	scanf("%u", &kvd_num_bits);

	printf("\n\n");

	kvd_result = kvd_a >> kvd_num_bits;

	PrintBinaryFormOfNumber(kvd_a);
	PrintBinaryFormOfNumber(kvd_result);

	printf("Thus, (%u >> %u) yields %u\n\n", kvd_a, kvd_num_bits, kvd_result);

	return(0);
}

// implementation of PrintBinaryFormOfNumber()
void PrintBinaryFormOfNumber(unsigned int kvd_decimal_number)
{
	// variable declarations
	unsigned kvd_remainder, kvd_binary[8], kvd_num, kvd_i;

	// code
	for (kvd_i = 0; kvd_i < 8; kvd_i++)
		kvd_binary[kvd_i] = 0;

	kvd_i = 7;
	kvd_num = kvd_decimal_number;

	while (kvd_num != 0)
	{
		kvd_remainder = kvd_num % 2;
		kvd_binary[kvd_i--] = kvd_remainder;
		kvd_num = kvd_num / 2;
	}

	printf("binary(%u)\t= ", kvd_decimal_number);
	for (kvd_i = 0; kvd_i < 8; kvd_i++)
		printf("%u", kvd_binary[kvd_i]);

	printf("\n\n");
}
