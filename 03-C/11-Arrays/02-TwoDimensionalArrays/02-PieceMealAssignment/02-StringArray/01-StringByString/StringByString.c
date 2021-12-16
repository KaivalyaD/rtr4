#include <stdio.h>

#define MAX_STRING_LENGTH 512

int main(void)
{
	// function prototypes
	void MyStrcpy(char[], char[]);

	// variable declarations
	char kvd_strArray[5][10];
	int kvd_strArray_size, kvd_strArray_row_count, kvd_strArray_col_count, kvd_strArray_element_count;
	int kvd_i;

	// code
	printf("\n\n");

	kvd_strArray_size = sizeof(kvd_strArray);
	printf("size of the 2D character array, kvd_strArray[][] = %d\n", kvd_strArray_size);

	kvd_strArray_row_count = kvd_strArray_size / sizeof(kvd_strArray[0]);
	printf("max. number of rows in kvd_strArray[][] = %d\n", kvd_strArray_row_count);

	kvd_strArray_col_count = sizeof(kvd_strArray[0]) / sizeof(kvd_strArray[0][0]);
	printf("max. number of columns in kvd_strArray[][] = %d\n", kvd_strArray_col_count);

	kvd_strArray_element_count = kvd_strArray_row_count * kvd_strArray_col_count;
	printf("max. number of elements in kvd_strArray[][] = %d\n", kvd_strArray_element_count);

	/* piece-meal assignment */
	MyStrcpy(kvd_strArray[0], "My");
	MyStrcpy(kvd_strArray[1], "name");
	MyStrcpy(kvd_strArray[2], "is");
	MyStrcpy(kvd_strArray[3], "Kaivalya");
	MyStrcpy(kvd_strArray[4], "Deshpande");

	printf("\n\n");

	printf("displaying all strings in kvd_strArray[][]:\n\n\t");
	for (kvd_i = 0; kvd_i < kvd_strArray_row_count; kvd_i++)
		printf("%s ", kvd_strArray[kvd_i]);

	printf("\n\n");
	return(0);
}

void MyStrcpy(char kvd_str_destination[], char kvd_str_source[])
{
	// function prototypes
	int MyStrlen(char[]);

	// variable declarations
	int kvd_source_length = 0, kvd_i;

	// code
	kvd_source_length = MyStrlen(kvd_str_source);
	
	for (kvd_i = 0; kvd_i < kvd_source_length; kvd_i++)
		kvd_str_destination[kvd_i] = kvd_str_source[kvd_i];

	kvd_str_destination[kvd_i] = '\0';
}

int MyStrlen(char kvd_str[])
{
	// variable declarations
	int kvd_string_length = 0;

	// code
	while (kvd_str[kvd_string_length] != '\0')
		kvd_string_length++;

	return(kvd_string_length);
}

