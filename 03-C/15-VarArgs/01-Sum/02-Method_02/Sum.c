#include <stdio.h>
#include <stdarg.h>

int main(void)
{
	// function prototypes
	int Sum(int, ...);

	// variable declarations
	int kvd_answer;

	// code
	printf("\n\n");

	kvd_answer = Sum(5, 1, 2, 3, 4, 5);
	printf("Sum(5, 1, 2, 3, 4, 5) = %d\n\n", kvd_answer);

	kvd_answer = Sum(3, 2, 4, 6);
	printf("Sum(3, 2, 4, 6) = %d\n\n", kvd_answer);

	kvd_answer = Sum(1, 8);
	printf("Sum(1, 8) = %d\n\n", kvd_answer);

	return 0;
}

int Sum(int kvd_argc, ...)
{
	// function prototypes
	int va_Sum(int, va_list);

	// variable declarations
	int kvd_sum = 0;
	va_list kvd_num_list;

	// code
	va_start(kvd_num_list, kvd_argc);
	kvd_sum = va_Sum(kvd_argc, kvd_num_list);
	va_end(kvd_num_list);

	return kvd_sum;
}

int va_Sum(int kvd_argc, va_list kvd_num_list)
{
	// variable declarations
	int kvd_sum = 0;

	// code
	while (kvd_argc)
	{
		kvd_sum = kvd_sum + va_arg(kvd_num_list, int);
		kvd_argc--;
	}

	return kvd_sum;
}
