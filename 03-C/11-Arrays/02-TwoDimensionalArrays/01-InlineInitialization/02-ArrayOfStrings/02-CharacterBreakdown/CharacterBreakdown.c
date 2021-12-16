#include <stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function prototype
	int MyStrlen(char[]);

	// variable declarations
	char kvd_strArray[10][15] = { "Hello!", "Welcome", "To", "Real", "Time", "Rendering", "Batch", "2021-22", "of", "ASTROMEDICOMP" };
	int kvd_iStrLengths[10];
	int kvd_strArray_size, kvd_strArray_row_count, kvd_i, kvd_j;

	// code
	printf("\n\n");

	kvd_strArray_size = sizeof(kvd_strArray);
	kvd_strArray_row_count = kvd_strArray_size / sizeof(kvd_strArray[0]);

	for (kvd_i = 0; kvd_i < kvd_strArray_row_count; kvd_i++)
		kvd_iStrLengths[kvd_i] = MyStrlen(kvd_strArray[kvd_i]);

	printf("displaying the entire kvd_strArray[][]:\n\n\t");
	for (kvd_i = 0; kvd_i < kvd_strArray_row_count; kvd_i++)
		printf("%s ", kvd_strArray[kvd_i]);

	printf("\n\n");

	printf("kvd_strArray[][] character breakdown:\n\n");
	for (kvd_i = 0; kvd_i < kvd_strArray_row_count; kvd_i++)
	{
		printf("\tstring at index %d: \"%s\"\n\n", kvd_i, kvd_strArray[kvd_i]);

		for (kvd_j = 0; kvd_j < kvd_iStrLengths[kvd_i]; kvd_j++)
			printf("\t\tcharacter at index %d = '%c'\n", kvd_j, kvd_strArray[kvd_i][kvd_j]);

		printf("\t\tcharacter at index %d = '\\0'\n\n", kvd_j);
	}

	return(0);
}

int MyStrlen(char kvd_str[])
{
	// variable declarations
	int kvd_i = 0, kvd_string_length;

	// code
	while (kvd_str[kvd_i] != '\0')
		kvd_i++;

	kvd_string_length = kvd_i;

	return(kvd_string_length);
}
