#include <stdio.h>

int main(void)
{
	// function prototypes
	void PrintBinaryFormOfNumber(int);

	// variable declarations
	int kvd_a, kvd_result;

	// code
	printf("\n\nenter kvd_a (int): ");
	scanf("%d", &kvd_a);

	printf("\n\n");

	kvd_result = ~(kvd_a);

	PrintBinaryFormOfNumber(kvd_a);
	PrintBinaryFormOfNumber(kvd_result);

	printf("Thus, ~(%d) yields %d\n\n", kvd_a, kvd_result);

	return(0);
}

// implementation of PrintBinaryFormOfNumber()
void PrintBinaryFormOfNumber(int kvd_decimal_number)
{
	// variable declarations
	unsigned kvd_remainder, kvd_binary[8 * sizeof(unsigned int)], kvd_num, kvd_i;

	// code
	for (kvd_i = 0; kvd_i < 8 * sizeof(unsigned int); kvd_i++)
		kvd_binary[kvd_i] = 0;

	kvd_i = (8 * sizeof(unsigned int)) - 1;
	kvd_num = kvd_decimal_number;

	while (kvd_num != 0)
	{
		kvd_remainder = kvd_num % 2;
		kvd_binary[kvd_i--] = kvd_remainder;
		kvd_num = kvd_num / 2;
	}

	printf("binary(%d)\t= ", kvd_decimal_number);
	for (kvd_i = 0; kvd_i < 8 * sizeof(unsigned int); kvd_i++)
		printf("%u", kvd_binary[kvd_i]);

	printf("\n\n");
}
