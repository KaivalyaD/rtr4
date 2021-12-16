#include <stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	/*
	 * A string is nothing else than an array of characters, i.e., of type char[].
	 * 
	 * Thus, an array of strings is an array of arrays of characters, or, 2D a array of characters, i.e. of type char[][]!
	 */

	// declaring an array of 10 strings, each having a maximum length of 15 characters (this size also considers the null terminator)
	char kvd_strArray[10][15] = { "Hello!", "Welcome", "To", "Real", "Time", "Rendering", "Batch", "2021-22", "of", "ASTROMEDICOMP" };  // inline initialization
	int kvd_strArray_size, kvd_strArray_row_count, kvd_strArray_col_count, kvd_strArray_ActualCharCount = 0, kvd_strArray_MaxCharCount;
	int kvd_i;
	
	// code
	printf("\n\n");

	kvd_strArray_size = sizeof(kvd_strArray);
	printf("size of the array of strings, kvd_strArray[][] = %d\n", kvd_strArray_size);

	kvd_strArray_row_count = kvd_strArray_size / sizeof(kvd_strArray[0]);
	printf("max. number of rows in kvd_strArray[][] = %d\n", kvd_strArray_row_count);
	
	kvd_strArray_col_count = sizeof(kvd_strArray[0]) / sizeof(kvd_strArray[0][0]);
	printf("max. number of columns in kvd_strArray[][] = %d\n", kvd_strArray_col_count);

	kvd_strArray_MaxCharCount = kvd_strArray_row_count * kvd_strArray_col_count;
	printf("max. number of characters that kvd_strArray[][] could hold = %d\n", kvd_strArray_MaxCharCount);

	for (kvd_i = 0; kvd_i < kvd_strArray_row_count; kvd_i++)
		kvd_strArray_ActualCharCount = kvd_strArray_ActualCharCount + MyStrlen(kvd_strArray[kvd_i]);
	printf("number of characters that kvd_strArray[][] holds currently = %d\n", kvd_strArray_ActualCharCount);

	printf("\n\n");

	printf("all strings (space separated) in kvd_strArray[][]:\n\n");
	printf("\t%s ", kvd_strArray[0]);
	printf("%s ", kvd_strArray[1]);
	printf("%s ", kvd_strArray[2]);
	printf("%s ", kvd_strArray[3]);
	printf("%s ", kvd_strArray[4]);
	printf("%s ", kvd_strArray[5]);
	printf("%s ", kvd_strArray[6]);
	printf("%s ", kvd_strArray[7]);
	printf("%s ", kvd_strArray[8]);
	printf("%s", kvd_strArray[9]);

	printf("\n\n");
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
