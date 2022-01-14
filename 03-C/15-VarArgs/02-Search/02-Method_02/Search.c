#include <stdio.h>
#include <stdarg.h>

#define FIND 4
#define COUNT_ARGC 10

int main(void)
{
	// function prototypes
	int Search(int, int, ...);

	// variable declarations
	int kvd_occurence_count;

	// code
	printf("\n\n");
	kvd_occurence_count = Search(FIND, COUNT_ARGC, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20);
	printf("%d occurred %d times in {2, 4, 6, 8, 10, 12, 14, 16, 18, 20}\n\n", FIND, kvd_occurence_count);

	return 0;
}

int Search(int kvd_find, int kvd_argc, ...)
{
	// function prototypes
	int va_Search(int, int, va_list);

	// variable declarations
	int kvd_count;
	va_list kvd_num_list;

	// code
	va_start(kvd_num_list, kvd_argc);
	kvd_count = va_Search(kvd_find, kvd_argc, kvd_num_list);
	va_end(kvd_num_list);

	return kvd_count;
}

int va_Search(int kvd_find, int kvd_argc, va_list kvd_num_list)
{
	// variable declarations
	int kvd_count = 0, kvd_i;

	while (kvd_argc)
	{
		if (va_arg(kvd_num_list, int) == kvd_find)
			kvd_count++;
		kvd_argc--;
	}

	return kvd_count;
}
