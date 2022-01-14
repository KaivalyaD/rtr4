#include <stdio.h>
#include <stdarg.h>

#define FIND 10
#define COUNT_ARGS 10

int main(void)
{
	// function prototypes
	int Search(int, int, ...);

	// variable declarations
	int kvd_occurence_count;

	// code
	printf("\n\n");

	kvd_occurence_count = Search(FIND, COUNT_ARGS, 5, 3, 4, 8, 9, 0, 1, 2, 3, 9);
	printf("%d occurred %d times in {5, 3, 4, 8, 9, 0, 1, 2, 3, 9}\n\n", FIND, kvd_occurence_count);

	return 0;
}

int Search(int kvd_find, int kvd_argc, ...)
{
	// variable declarations
	int kvd_count = 0, kvd_i;
	va_list kvd_num_list;

	// code
	va_start(kvd_num_list, kvd_argc);
	while (kvd_argc)
	{
		if (va_arg(kvd_num_list, int) == kvd_find)
			kvd_count++;
		kvd_argc--;
	}
	va_end(kvd_num_list);

	return kvd_count;
}
